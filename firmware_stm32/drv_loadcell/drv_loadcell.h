/**
 * @file drv_loadcell.h
 * @brief HX711 Load Cell Driver for STM32 using HAL Library
 */

#ifndef DRV_LOADCELL_H
#define DRV_LOADCELL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h" // Change to your specific HAL header

// Configuration
#define LC_TIMEOUT_US   1000
#define LC_ERROR_CODE   -2147483648
#define HX711_SIGN_MASK 0xFF000000

typedef struct {
    GPIO_TypeDef *dout_port; // Data Out Port
    uint16_t dout_pin;       // Data Out Pin
    GPIO_TypeDef *sck_port;  // Clock Port
    uint16_t sck_pin;        // Clock Pin
    
    bool is_initialized;
    int32_t offset;          // Tare value
    float scale;             // Calibration factor
} loadcell_t;

// Function Prototypes

/**
 * @brief Enables DWT cycle counter for microsecond delay accuracy.
 * Must be called once before using the driver.
 */
void loadcell_init_dwt(void);

/**
 * @brief Initializes the load cell struct. 
 * Note: GPIO Mode/Pull-up must be configured via CubeMX/MX_GPIO_Init first.
 */
HAL_StatusTypeDef loadcell_init(loadcell_t *sensor, 
                                GPIO_TypeDef *dout_port, uint16_t dout_pin,
                                GPIO_TypeDef *sck_port, uint16_t sck_pin);

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
float loadcell_get_weight(loadcell_t *sensor);

/**
 * @brief Manually set the scale factor.
 */
void loadcell_set_scale(loadcell_t *sensor, float scale_value);

#ifdef __cplusplus
}
#endif

#endif // DRV_LOADCELL_H
