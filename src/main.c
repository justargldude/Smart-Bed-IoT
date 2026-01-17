/**
 * @file main.c
 * @brief SmartCrib main application - ESP32-S3 health monitoring system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "app_config.h"
#include "drv_loadcell.h"
#include "drv_mpu.h"

static const char *TAG = "MAIN";

// Global Variables
loadcell_t sensor_front_left;
loadcell_t sensor_front_right;
loadcell_t sensor_back_left;
loadcell_t sensor_back_right;

MPU9250_t myMpu;

// SPI Configuration Function
void init_spi_bus(void) {
    spi_bus_config_t buscfg = {
        .miso_io_num = MPU_PIN_NUM_MISO,
        .mosi_io_num = MPU_PIN_NUM_MOSI,
        .sclk_io_num = MPU_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    
    // Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(MPU_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Add MPU9250 to SPI bus
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,  // 1 MHz
        .mode = 0,                          // SPI mode 0
        .spics_io_num = -1,                 // Manually control CS
        .queue_size = 7,
    };
    
    ESP_ERROR_CHECK(spi_bus_add_device(MPU_SPI_HOST, &devcfg, &myMpu.spi_handle));
    
    // Configure CS pin manually
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << MPU_PIN_NUM_CS),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);
    
    myMpu.cs_pin = MPU_PIN_NUM_CS;
    gpio_set_level(myMpu.cs_pin, 1); // Set CS High initially
}

void app_main(void)
{
    // Initialize SPI
    init_spi_bus();
    ESP_LOGI(TAG, "System Starting");

    // Initialize MPU
    if (mpu_init(&myMpu) == ESP_OK) {
        ESP_LOGI(TAG, "MPU Init: OK (Chip Found)");
    } else {
        ESP_LOGI(TAG, "MPU Init: FAILED (Check Wiring)");
    }

    // Initialize 4 load cells at bed corners
    loadcell_init(&sensor_front_left, FRONT_LEFT_DT_PIN, FRONT_LEFT_SCK_PIN);
    loadcell_init(&sensor_front_right, FRONT_RIGHT_DT_PIN, FRONT_RIGHT_SCK_PIN);
    loadcell_init(&sensor_back_left, BACK_LEFT_DT_PIN, BACK_LEFT_SCK_PIN);
    loadcell_init(&sensor_back_right, BACK_RIGHT_DT_PIN, BACK_RIGHT_SCK_PIN);

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Setup scale factors
    sensor_front_left.offset = 8432156;
    sensor_front_left.scale = 420.5f;
  
    sensor_front_right.offset = 8431200;
    sensor_front_right.scale = 418.3f;
  
    sensor_back_left.offset = 8433500;
    sensor_back_left.scale = 422.1f;
  
    sensor_back_right.offset = 8430800;
    sensor_back_right.scale = 419.7f;

    // Main Loop
    while (1)
    {
        uint8_t whoami;
        esp_err_t ret;

        // Check MPU ID
        ret = spi_read_byte(&myMpu, MPU_REG_WHO_AM_I, &whoami);

        if (ret == ESP_OK && whoami == 0x70) {
            // Uncomment to debug connection status
        } else {
            printf("MPU DISCONNECTED! ID: 0x%02X\n", whoami);
        }

        // Read MPU Data
        mpu_read_all(&myMpu);
        
        // Print CSV for SerialPlot
        // Format: Accel_X, Accel_Y, Accel_Z
        printf("%ld,%ld,%ld\n", myMpu.accel_mg[0], myMpu.accel_mg[1], myMpu.accel_mg[2]);
        
        // Delay 20ms ~ 50Hz sample rate
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void Error_Handler(void)
{
    while (1) { vTaskDelay(100); }
}