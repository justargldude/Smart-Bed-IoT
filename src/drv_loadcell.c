#include "drv_loadcell.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"

// Biến này dùng để khóa ngắt (Critical Section)
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

esp_err_t loadcell_init(loadcell_t *sensor, gpio_num_t pin_dout, gpio_num_t pin_sck) {
    sensor->pin_dout = pin_dout;
    sensor->pin_sck = pin_sck;
    sensor->offset = 0;
    sensor->scale = 1.0f;

    // Config DOUT (Input)
    gpio_config_t dout_conf = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << sensor->pin_dout),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE, // [SỬA] Nên bật Pull-up để tín hiệu ổn định
    };
    if (gpio_config(&dout_conf) != ESP_OK) return ESP_FAIL;

    // Config SCK (Output)
    gpio_config_t sck_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << sensor->pin_sck),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    if (gpio_config(&sck_conf) != ESP_OK) return ESP_FAIL;

    // Reset SCK mức thấp
    gpio_set_level(sensor->pin_sck, 0);
    sensor->is_initialized = true;
    
    return ESP_OK;
}

int32_t loadcell_read_raw(loadcell_t *sensor) {
    if (!sensor->is_initialized) return LC_ERROR_CODE;

    // 1. Chờ DOUT xuống thấp (Data Ready)
    // Dùng biến cục bộ timeout để đếm lùi
    int timeout = TIME_OUT_US;
    while (gpio_get_level(sensor->pin_dout)) {
        ets_delay_us(1);
        if (--timeout <= 0) return LC_ERROR_CODE;
    }

    uint32_t value = 0;

    portENTER_CRITICAL(&mux);

    // 2. Đọc 24 bit (MSB First - Bit cao nhất đến trước)
    for (int i = 0; i < 24; i++) {
        gpio_set_level(sensor->pin_sck, 1);
        ets_delay_us(1); // Giữ xung cao tối thiểu 0.2us (HX711 spec)

        // Dịch bit cũ sang trái 1 đơn vị để đón bit mới vào vị trí LSB
        value = value << 1; 

        gpio_set_level(sensor->pin_sck, 0);
        
        // Nếu chân DOUT mức cao thì cộng 1 vào bit cuối cùng
        if (gpio_get_level(sensor->pin_dout)) {
            value++; 
        }
        
        ets_delay_us(1); // Giữ xung thấp
    }

    // 3. Xung thứ 25 (Đặt Gain 128 cho lần đọc sau)
    gpio_set_level(sensor->pin_sck, 1);
    ets_delay_us(1);
    gpio_set_level(sensor->pin_sck, 0);
    ets_delay_us(1);

    portEXIT_CRITICAL(&mux);

    // 4. Xử lý số âm (Sign Extension)
    if (value & 0x800000) {
        value |= HX711_SIGN_MASK;
    }

    return (int32_t)value;
}


float loadcell_get_weight(loadcell_t *sensor) {
    int32_t raw = loadcell_read_raw(sensor);
    if (raw == LC_ERROR_CODE) return 0.0f;
    
    // Công thức: (Raw - Offset) / Scale
    return (float)(raw - sensor->offset) / sensor->scale;
}