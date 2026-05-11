#include "drv_mpu.h"
#include <string.h>

esp_err_t spi_write_byte(MPU9250_t *dev, uint8_t regAddress, uint8_t data)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    regAddress &= 0x7F;

    uint8_t tx_data[2] = {regAddress, data};

    t.length = 8 * 2;
    t.tx_buffer = tx_data;

    gpio_set_level(dev->cs_pin, 0);
    ret = spi_device_transmit(dev->spi_handle, &t);
    gpio_set_level(dev->cs_pin, 1);

    return ret;
}

esp_err_t spi_read_byte(MPU9250_t *dev, uint8_t regAddress, uint8_t *data)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    regAddress |= MPU_READ;

    uint8_t tx_data[2] = {regAddress, 0x00};
    uint8_t rx_data[2] = {0};

    t.length = 8 * 2;
    t.tx_buffer = tx_data;
    t.rx_buffer = rx_data;

    gpio_set_level(dev->cs_pin, 0);
    ret = spi_device_transmit(dev->spi_handle, &t);
    gpio_set_level(dev->cs_pin, 1);

    if (ret == ESP_OK) {
        *data = rx_data[1];
    }

    return ret;
}

esp_err_t spi_burst_read(MPU9250_t *dev, uint8_t regAddress, uint8_t *buffer, uint16_t length)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    regAddress |= MPU_READ;

    uint8_t *tx_buf = heap_caps_malloc(length + 1, MALLOC_CAP_DMA);
    uint8_t *rx_buf = heap_caps_malloc(length + 1, MALLOC_CAP_DMA);

    if (!tx_buf || !rx_buf) {
        if (tx_buf) free(tx_buf);
        if (rx_buf) free(rx_buf);
        return ESP_ERR_NO_MEM;
    }

    memset(tx_buf, 0, length + 1);
    tx_buf[0] = regAddress;

    t.length = 8 * (length + 1);
    t.tx_buffer = tx_buf;
    t.rx_buffer = rx_buf;

    gpio_set_level(dev->cs_pin, 0);
    ret = spi_device_transmit(dev->spi_handle, &t);
    gpio_set_level(dev->cs_pin, 1);

    if (ret == ESP_OK) {
        memcpy(buffer, &rx_buf[1], length);
    }

    free(tx_buf);
    free(rx_buf);

    return ret;
}

static esp_err_t check_connection(MPU9250_t *dev)
{
    esp_err_t status;
    uint8_t whoami;
    status = spi_read_byte(dev, MPU_REG_WHO_AM_I, &whoami);
    if (status != ESP_OK) return status;

    if (whoami != MPU_WHO_AM_I_VALUE) return ESP_FAIL;
    return ESP_OK;
}

esp_err_t mpu_init(MPU9250_t *dev)
{
    dev->accel_sens = ACCEL_SENSITIVITY;
    dev->gyro_sens = GYRO_SENSITIVITY;
    dev->data_ready = false;
    for (int i = 0; i < 3; i++) {
        dev->accel_ma[i] = 0;
    }

    esp_err_t status = check_connection(dev);
    if (status != ESP_OK) return status;

    status = spi_write_byte(dev, MPU_REG_PWR_MGMT_1, RESET_DEVICE);
    if (status != ESP_OK) return status;
    vTaskDelay(pdMS_TO_TICKS(100));

    status = spi_write_byte(dev, MPU_REG_PWR_MGMT_1, WAKE_UP);
    if (status != ESP_OK) return status;
    vTaskDelay(pdMS_TO_TICKS(10));

    status = spi_write_byte(dev, MPU_REG_ACCEL_CONFIG, CONFIG_ACCEL);
    if (status != ESP_OK) return status;

    status = spi_write_byte(dev, MPU_REG_GYRO_CONFIG, CONFIG_GYRO);
    if (status != ESP_OK) return status;

    status = spi_write_byte(dev, MPU_REG_CONFIG, BANDWIDTH);
    if (status != ESP_OK) return status;

    status = spi_write_byte(dev, MPU_REG_ACCEL_CONFIG_2, BANDWIDTH);
    if (status != ESP_OK) return status;

    status = spi_write_byte(dev, MPU_REG_SMPLRT_DIV, SMPLRT_DIV);
    if (status != ESP_OK) return status;

    return ESP_OK;
}

esp_err_t mpu_read_all(MPU9250_t *dev)
{
    uint8_t buffer[BUFFER_SIZE];

    if (spi_burst_read(dev, MPU_REG_ACCEL_XOUT_H, buffer, BUFFER_SIZE) != ESP_OK) {
        return ESP_FAIL;
    }

    dev->accel_raw[0] = (int16_t)(buffer[0] << 8 | buffer[1]);
    dev->accel_raw[1] = (int16_t)(buffer[2] << 8 | buffer[3]);
    dev->accel_raw[2] = (int16_t)(buffer[4] << 8 | buffer[5]);

    for (int i = 0; i < 3; i++) {
        int64_t temp_a = (int64_t)dev->accel_raw[i] * 1000;
        dev->accel_mg[i] = (int32_t)(temp_a / dev->accel_sens);
    }

    return ESP_OK;
}

void moving_average(MPU9250_t *dev)
{
    for (int i = 0; i < 3; i++) {
        dev->accel_ma[i] = (int32_t)(ALPHA * dev->accel_mg[i] + (1.0f - ALPHA) * dev->accel_ma[i]);
    }
    dev->data_ready = true;
}