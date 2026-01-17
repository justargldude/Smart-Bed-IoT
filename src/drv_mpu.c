/**
 * @file drv_mpu.c
 * @brief MPU9250 IMU driver via SPI for ESP32
 */

#include "drv_mpu.h"
#include <string.h>

// === LOW-LEVEL SPI OPERATIONS ===

esp_err_t spi_write_byte(MPU9250_t *dev, uint8_t regAddress, uint8_t data) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    regAddress &= 0x7F;  // Ensure WRITE mode (bit 7 = 0)
    
    // Prepare data: Address first, then Data
    uint8_t tx_data[2] = {regAddress, data};

    t.length = 8 * 2;           // Total bits
    t.tx_buffer = tx_data;
    
    // Manual CS Control
    gpio_set_level(dev->cs_pin, 0);
    ret = spi_device_transmit(dev->spi_handle, &t);
    gpio_set_level(dev->cs_pin, 1);
    
    return ret;
}

esp_err_t spi_read_byte(MPU9250_t *dev, uint8_t regAddress, uint8_t *data) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    
    regAddress |= MPU_READ;  // Set READ mode (bit 7 = 1)

    
    uint8_t tx_data[2] = {regAddress, 0x00};
    uint8_t rx_data[2] = {0};

    t.length = 8 * 2;
    t.tx_buffer = tx_data;
    t.rx_buffer = rx_data;
    
    gpio_set_level(dev->cs_pin, 0);
    ret = spi_device_transmit(dev->spi_handle, &t);
    gpio_set_level(dev->cs_pin, 1);
    
    if (ret == ESP_OK) {
        *data = rx_data[1]; // Second byte is the returned data
    }
    
    return ret;
}

esp_err_t spi_burst_read(MPU9250_t *dev, uint8_t regAddress, uint8_t *buffer, uint16_t length) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    
    regAddress |= MPU_READ;
    
    // Allocate DMA-capable buffers for SPI transaction
    // TX and RX buffers must be equal size for ESP32 SPI
    uint8_t *tx_buf = heap_caps_malloc(length + 1, MALLOC_CAP_DMA);
    uint8_t *rx_buf = heap_caps_malloc(length + 1, MALLOC_CAP_DMA);
    
    if (!tx_buf || !rx_buf) {
        if (tx_buf) free(tx_buf);
        if (rx_buf) free(rx_buf);
        return ESP_ERR_NO_MEM;
    }

    memset(tx_buf, 0, length + 1);
    tx_buf[0] = regAddress; // First byte is register address

    t.length = 8 * (length + 1);
    t.tx_buffer = tx_buf;
    t.rx_buffer = rx_buf;
    
    gpio_set_level(dev->cs_pin, 0);
    ret = spi_device_transmit(dev->spi_handle, &t);
    gpio_set_level(dev->cs_pin, 1);
    
    if (ret == ESP_OK) {
        memcpy(buffer, &rx_buf[1], length); // Copy data from second byte onwards
    }

    free(tx_buf);
    free(rx_buf);
    
    return ret;
}

esp_err_t check_connection(MPU9250_t *dev){
    esp_err_t status;
    uint8_t whoami;
    status = spi_read_byte(dev, MPU_REG_WHO_AM_I, &whoami);
    if(status != ESP_OK) return status;
    
    // printf("ID: 0x%02X\n", whoami); // Debug
    if(whoami != MPU_WHO_AM_I_VALUE) return ESP_FAIL;
    return ESP_OK;
}

esp_err_t mpu_init(MPU9250_t *dev){
    // SPI handle and CS pin are assigned in main
    
    // Set sensitivity dividers
    dev->accel_sens = ACCEL_SENSITIVITY;
    dev->gyro_sens = GYRO_SENSITIVITY;
    
    esp_err_t status;
    status = check_connection(dev);
    if(status != ESP_OK) return status;
    
    status = spi_write_byte(dev, MPU_REG_PWR_MGMT_1, RESET_DEVICE);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    status = spi_write_byte(dev, MPU_REG_PWR_MGMT_1, WAKE_UP);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    status = spi_write_byte(dev, MPU_REG_ACCEL_CONFIG, CONFIG_ACCEL);
    status = spi_write_byte(dev, MPU_REG_GYRO_CONFIG, CONFIG_GYRO);
    status = spi_write_byte(dev, MPU_REG_CONFIG, BANDWIDTH);
    status = spi_write_byte(dev, MPU_REG_ACCEL_CONFIG_2, BANDWIDTH);
    status = spi_write_byte(dev, MPU_REG_SMPLRT_DIV, SMPLRT_DIV);
    
    return status;
}

esp_err_t mpu_read_all(MPU9250_t *dev){
    uint8_t buffer[BUFFER_SIZE]; // 14 bytes
    
    if(spi_burst_read(dev, MPU_REG_ACCEL_XOUT_H, buffer, BUFFER_SIZE) != ESP_OK){
        return ESP_FAIL;
    }
    
    // Accel: Byte 0-5
    dev->accel_raw[0] = (int16_t)(buffer[0] << 8 | buffer[1]); 
    dev->accel_raw[1] = (int16_t)(buffer[2] << 8 | buffer[3]); 
    dev->accel_raw[2] = (int16_t)(buffer[4] << 8 | buffer[5]); 

    // Gyro: Byte 8-13
    dev->gyro_raw[0]  = (int16_t)(buffer[8] << 8 | buffer[9]);  
    dev->gyro_raw[1]  = (int16_t)(buffer[10] << 8 | buffer[11]); 
    dev->gyro_raw[2]  = (int16_t)(buffer[12] << 8 | buffer[13]); 

    for(int i = 0; i < 3; i++){
        int64_t temp_a = (int64_t)dev->accel_raw[i] * 1000;
        dev->accel_mg[i] = (int32_t)(temp_a / dev->accel_sens);
        
        int64_t temp_g = (int64_t)dev->gyro_raw[i] * 1000;
        dev->gyro_mdps[i] = (int32_t)(temp_g / dev->gyro_sens);
    }
    return ESP_OK;
}