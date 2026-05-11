#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/gpio.h"
#include "esp_err.h"

#define FRONT_LEFT_SCK_PIN  GPIO_NUM_1
#define FRONT_LEFT_DT_PIN   GPIO_NUM_2

#define FRONT_RIGHT_SCK_PIN GPIO_NUM_42
#define FRONT_RIGHT_DT_PIN  GPIO_NUM_41

#define BACK_LEFT_SCK_PIN   GPIO_NUM_40
#define BACK_LEFT_DT_PIN    GPIO_NUM_39

#define BACK_RIGHT_SCK_PIN  GPIO_NUM_38
#define BACK_RIGHT_DT_PIN   GPIO_NUM_37

#define MPU_SPI_HOST        SPI2_HOST
#define MPU_PIN_NUM_MISO    GPIO_NUM_13
#define MPU_PIN_NUM_MOSI    GPIO_NUM_11
#define MPU_PIN_NUM_CLK     GPIO_NUM_12
#define MPU_PIN_NUM_CS      GPIO_NUM_10

#define PRESENCE_THRESHOLD_KG   5
#define PRESENCE_DEBOUNCE_COUNT 3

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif