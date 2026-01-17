/**
 * @file drv_loadcell.c
 * @brief HX711 Load Cell driver for ESP32
 */

#include "drv_loadcell.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Spinlock to protect critical bit-banging section
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

static inline void delay_us(uint32_t us) 
{
    esp_rom_delay_us(us);
}

esp_err_t loadcell_init(loadcell_t *sensor, gpio_num_t dout_pin, gpio_num_t sck_pin) 
{
    sensor->dout_pin = dout_pin;
    sensor->sck_pin = sck_pin;
    sensor->offset = 0;
    sensor->scale = 1.0f;
    sensor->is_initialized = true;

    // Configure GPIO
    gpio_config_t io_conf = {};
    
    // Config DOUT as Input
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << dout_pin);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0; // HX711 has external pull-up or not needed
    gpio_config(&io_conf);

    // Config SCK as Output
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << sck_pin);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    // Ensure SCK is low to prevent entering power-down mode unexpectedly
    gpio_set_level(sensor->sck_pin, 0);
    
    return ESP_OK;
}

int32_t loadcell_read_raw(loadcell_t *sensor) 
{
    if (!sensor->is_initialized) return LC_ERROR_CODE;

    // 1. Wait for DOUT to go low (Data Ready)
    int16_t timeout = LC_TIMEOUT_US;
    while (gpio_get_level(sensor->dout_pin) == 1) 
    {
        delay_us(1);
        if (--timeout <= 0) return LC_ERROR_CODE;
    }

    uint32_t value = 0;

    // 2. Critical Section: Disable interrupts/scheduling on this core
    portENTER_CRITICAL(&spinlock);

    // 3. Read 24 bits (MSB First)
    for (int i = 0; i < 24; i++) 
    {
        // Clock High
        gpio_set_level(sensor->sck_pin, 1);
        delay_us(1);

        // Shift buffer
        value = value << 1;

        // Sample DOUT
        if (gpio_get_level(sensor->dout_pin)) 
        {
            value++;
        }

        // Clock Low
        gpio_set_level(sensor->sck_pin, 0);
        delay_us(1);
    }

    // 4. 25th Pulse for Gain 128 / Channel A
    gpio_set_level(sensor->sck_pin, 1);
    delay_us(1);
    gpio_set_level(sensor->sck_pin, 0);
    delay_us(1);

    // 5. Exit Critical Section
    portEXIT_CRITICAL(&spinlock);

    // 6. Sign Extension (24-bit to 32-bit signed)
    if (value & (1UL << 23)) 
    {
        value |= HX711_SIGN_MASK;
    }

    return (int32_t)value;
}

int32_t loadcell_read_average(loadcell_t *sensor, uint8_t times) 
{
    if (times < 1) times = 1;
    
    int64_t sum = 0;
    uint8_t valid_count = 0;
    
    for (uint8_t i = 0; i < times; i++) 
    {
        int32_t raw = loadcell_read_raw(sensor);
        if (raw != LC_ERROR_CODE) 
        {
            sum += raw;
            valid_count++;
        }
        // Small delay
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (valid_count == 0) return LC_ERROR_CODE;
    return (int32_t)(sum / valid_count);
}

void loadcell_tare(loadcell_t *sensor) {
    int32_t avg = loadcell_read_average(sensor, 10);
    if (avg != LC_ERROR_CODE) {
        sensor->offset = avg;
    }
}

void loadcell_set_scale(loadcell_t *sensor, float scale_value) {
    sensor->scale = scale_value;
}

int16_t loadcell_get_weight(loadcell_t *sensor) {
    int32_t raw = loadcell_read_raw(sensor);
    if (raw == LC_ERROR_CODE) return 0;
    
    return (int16_t)(raw - sensor->offset)*1000 / sensor->scale;
}