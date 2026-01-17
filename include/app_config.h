#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/gpio.h"
#include "esp_err.h"

// Loadcell 1 (Front Left)
#define FRONT_LEFT_SCK_PIN  GPIO_NUM_1
#define FRONT_LEFT_DT_PIN   GPIO_NUM_2

// Loadcell 2 (Front Right)
#define FRONT_RIGHT_SCK_PIN GPIO_NUM_42
#define FRONT_RIGHT_DT_PIN  GPIO_NUM_41

// Loadcell 3 (Back Left)
#define BACK_LEFT_SCK_PIN   GPIO_NUM_40
#define BACK_LEFT_DT_PIN    GPIO_NUM_39

// Loadcell 4 (Back Right)
#define BACK_RIGHT_SCK_PIN  GPIO_NUM_38
#define BACK_RIGHT_DT_PIN   GPIO_NUM_37

// SPI MPU9250
#define MPU_SPI_HOST        SPI2_HOST // Use SPI2
#define MPU_PIN_NUM_MISO    GPIO_NUM_13
#define MPU_PIN_NUM_MOSI    GPIO_NUM_11
#define MPU_PIN_NUM_CLK     GPIO_NUM_12
#define MPU_PIN_NUM_CS      GPIO_NUM_10

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif // APP_CONFIG_H