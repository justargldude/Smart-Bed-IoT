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

// Configuration
#define LC_TIMEOUT_US   1000
#define LC_ERROR_CODE   -2147483648
#define HX711_SIGN_MASK 0xFF000000

typedef struct {
    gpio_num_t dout_pin;     // Data Out Pin
    gpio_num_t sck_pin;      // Clock Pin
    
    bool is_initialized;
    int32_t offset;          // Tare value
    int32_t scale;             // Calibration factor
} loadcell_t;

// Function Prototypes

/**
 * @brief Initializes the load cell struct and GPIOs.
 */
esp_err_t loadcell_init(loadcell_t *sensor, gpio_num_t dout_pin, gpio_num_t sck_pin);

/**
 * @brief Reads raw 24-bit value from HX711.
 * @return Raw value or LC_ERROR_CODE on timeout.
 */
int32_t loadcell_read_raw(loadcell_t *sensor);

/**
 * @brief Reads average value to reduce noise.
 */
int32_t loadcell_read_average(loadcell_t *sensor, uint8_t times);

/**
 * @brief Sets current weight as zero point (Tare).
 */
void loadcell_tare(loadcell_t *sensor);

/**
 * @brief Converts raw data to weight unit (kg/g/lb).
 */
int16_t loadcell_get_weight(loadcell_t *sensor);

/**
 * @brief Manually set the scale factor.
 */
void loadcell_set_scale(loadcell_t *sensor, float scale_value);

#ifdef __cplusplus
}
#endif

#endif // DRV_LOADCELL_H