#ifndef DRV_LOADCELL_H
#define DRV_LOADCELL_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h" // Thêm để dùng kiểu gpio_num_t

#define TIME_OUT_US 1000 // Đổi tên cho rõ là Microseconds
#define HX711_SIGN_MASK 0xFF000000
#define LC_ERROR_CODE   -2147483648

typedef struct {
    gpio_num_t pin_dout; // Dùng đúng kiểu dữ liệu của ESP-IDF
    gpio_num_t pin_sck;
    bool is_initialized;
    int32_t offset;      // Thêm biến offset để lưu giá trị bì (Tare)
    float scale;         // Thêm biến scale để lưu hệ số chia
} loadcell_t;

// Khai báo hàm
esp_err_t loadcell_init(loadcell_t *sensor, gpio_num_t pin_dout, gpio_num_t pin_sck);
int32_t loadcell_read_raw(loadcell_t *sensor);
float loadcell_get_weight(loadcell_t *sensor);
#endif