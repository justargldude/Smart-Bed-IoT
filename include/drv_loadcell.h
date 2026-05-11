#ifndef DRV_LOADCELL_H
#define DRV_LOADCELL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_rom_sys.h"

#define LC_TIMEOUT_US   1000
#define LC_ERROR_CODE   -2147483648
#define HX711_SIGN_MASK 0xFF000000

typedef struct {
    gpio_num_t dout_pin;
    gpio_num_t sck_pin;
    bool is_initialized;
    int32_t offset;
    float scale;
} loadcell_t;

esp_err_t loadcell_init(loadcell_t *sensor, gpio_num_t dout_pin, gpio_num_t sck_pin);
int32_t loadcell_read_raw(loadcell_t *sensor);
int32_t loadcell_read_average(loadcell_t *sensor, uint8_t times);
void loadcell_tare(loadcell_t *sensor);
int16_t loadcell_get_weight(loadcell_t *sensor);
void loadcell_set_scale(loadcell_t *sensor, float scale_value);

#ifdef __cplusplus
}
#endif

#endif