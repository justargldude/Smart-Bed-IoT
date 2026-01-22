#ifndef DRV_MPU_H
#define DRV_MPU_H

#include "app_config.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

// === CONSTANTS ===
#define ACCEL_SENSITIVITY 16384
#define GYRO_SENSITIVITY  131

// Burst read 14 byte (Accel 6 + Temp 2 + Gyro 6)
#define BUFFER_SIZE       14 

// === STRUCT ===
typedef struct {
    spi_device_handle_t spi_handle; // ESP-IDF SPI Device Handle
    gpio_num_t cs_pin;              // Chip Select Pin
    
    int16_t accel_raw[3]; // [0]:X, [1]:Y, [2]:Z
    int16_t gyro_raw[3];
    
    // Output in Fixed-Point format
    int32_t accel_mg[3];   // milli-g

    int32_t accel_ma[3];
    int32_t gyro_ma[3];
    bool data_ready;

    // Sensitivity for conversion
    uint16_t accel_sens;   
    uint16_t gyro_sens;
} MPU9250_t;

// === MACROS ===
// Note: CS PIN definition moved to main.h or struct init
#define MPU_TIME_OUT     100

#define MPU_REG_PWR_MGMT_1    0x6B
#define MPU_REG_ACCEL_CONFIG_2  0x1D
#define MPU_REG_ACCEL_CONFIG  0x1C
#define MPU_REG_GYRO_CONFIG   0x1B
#define MPU_REG_CONFIG        0x1A
#define MPU_REG_WHO_AM_I      0x75
#define MPU_REG_SMPLRT_DIV    0x19
#define MPU_REG_ACCEL_XOUT_H  0x3B 

#define MPU_WHO_AM_I_VALUE    0x70 
#define MPU_READ              0x80
#define RESET_DEVICE          0x80
#define WAKE_UP               0x01
#define CONFIG_ACCEL          0x00 // ±2g
#define CONFIG_GYRO           0x00 // ±250dps
#define BANDWIDTH             0x06
#define SMPLRT_DIV            0x09
#define ALPHA 0.1f

// === PROTOTYPES ===
esp_err_t mpu_init(MPU9250_t *dev);
esp_err_t spi_read_byte(MPU9250_t *dev, uint8_t regAddress, uint8_t *data);
esp_err_t mpu_read_all(MPU9250_t *dev); 
void moving_average(MPU9250_t *dev);

#endif // DRV_MPU_H