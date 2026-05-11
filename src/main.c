#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "app_config.h"
#include "drv_loadcell.h"
#include "drv_mpu.h"
#include "wifi_config.h"
#include "mqtt_config.h"

static const char *TAG = "MAIN";

loadcell_t sensor_front_left;
loadcell_t sensor_front_right;
loadcell_t sensor_back_left;
loadcell_t sensor_back_right;

MPU9250_t myMpu;

typedef struct {
    int16_t weight[4];
    int32_t accel_filtered[3];
    bool    person_present;
} shared_data_t;

static shared_data_t g_data = {0};
static SemaphoreHandle_t g_data_mutex = NULL;

static void init_spi_bus(void)
{
    spi_bus_config_t buscfg = {
        .miso_io_num = MPU_PIN_NUM_MISO,
        .mosi_io_num = MPU_PIN_NUM_MOSI,
        .sclk_io_num = MPU_PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(MPU_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 7,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(MPU_SPI_HOST, &devcfg, &myMpu.spi_handle));

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << MPU_PIN_NUM_CS),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);

    myMpu.cs_pin = MPU_PIN_NUM_CS;
    gpio_set_level(myMpu.cs_pin, 1);
}

static void task_sensor_read(void *pvParameters)
{
    int16_t local_weight[4];
    int32_t local_accel[3];

    while (1)
    {
        if (mpu_read_all(&myMpu) == ESP_OK) {
            moving_average(&myMpu);
            local_accel[0] = myMpu.accel_ma[0];
            local_accel[1] = myMpu.accel_ma[1];
            local_accel[2] = myMpu.accel_ma[2];
        }

        local_weight[0] = loadcell_get_weight(&sensor_front_left);
        local_weight[1] = loadcell_get_weight(&sensor_front_right);
        local_weight[2] = loadcell_get_weight(&sensor_back_left);
        local_weight[3] = loadcell_get_weight(&sensor_back_right);

        if (xSemaphoreTake(g_data_mutex, portMAX_DELAY) == pdTRUE)
        {
            memcpy(g_data.weight, local_weight, sizeof(local_weight));
            memcpy(g_data.accel_filtered, local_accel, sizeof(local_accel));
            xSemaphoreGive(g_data_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void task_process_publish(void *pvParameters)
{
    shared_data_t local;
    uint8_t presence_counter = 0;

    while (1)
    {
        if (xSemaphoreTake(g_data_mutex, portMAX_DELAY) == pdTRUE)
        {
            memcpy(&local, &g_data, sizeof(shared_data_t));
            xSemaphoreGive(g_data_mutex);
        }

        int32_t total_weight = local.weight[0] + local.weight[1]
                             + local.weight[2] + local.weight[3];

        if (total_weight > PRESENCE_THRESHOLD_KG) {
            if (presence_counter < PRESENCE_DEBOUNCE_COUNT) {
                presence_counter++;
            }
        } else {
            if (presence_counter > 0) {
                presence_counter--;
            }
        }

        bool detected = (presence_counter >= PRESENCE_DEBOUNCE_COUNT);

        if (xSemaphoreTake(g_data_mutex, portMAX_DELAY) == pdTRUE)
        {
            g_data.person_present = detected;
            xSemaphoreGive(g_data_mutex);
        }

        ESP_LOGI("PROC", "W:[%d,%d,%d,%d] T:%ld P:%s A:[%ld,%ld,%ld]",
            local.weight[0], local.weight[1],
            local.weight[2], local.weight[3],
            total_weight,
            detected ? "YES" : "NO",
            local.accel_filtered[0],
            local.accel_filtered[1],
            local.accel_filtered[2]);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    init_spi_bus();
    ESP_LOGI(TAG, "System Starting");

    if (mpu_init(&myMpu) == ESP_OK) {
        ESP_LOGI(TAG, "MPU Init: OK");
    } else {
        ESP_LOGE(TAG, "MPU Init: FAILED");
    }

    loadcell_init(&sensor_front_left, FRONT_LEFT_DT_PIN, FRONT_LEFT_SCK_PIN);
    loadcell_init(&sensor_front_right, FRONT_RIGHT_DT_PIN, FRONT_RIGHT_SCK_PIN);
    loadcell_init(&sensor_back_left, BACK_LEFT_DT_PIN, BACK_LEFT_SCK_PIN);
    loadcell_init(&sensor_back_right, BACK_RIGHT_DT_PIN, BACK_RIGHT_SCK_PIN);

    vTaskDelay(pdMS_TO_TICKS(1000));

    sensor_front_left.offset  = 8432156;  sensor_front_left.scale  = 420.5f;
    sensor_front_right.offset = 8431200;  sensor_front_right.scale = 418.3f;
    sensor_back_left.offset   = 8433500;  sensor_back_left.scale   = 422.1f;
    sensor_back_right.offset  = 8430800;  sensor_back_right.scale  = 419.7f;

    if (wifi_init_sta_with_provisioning() != ESP_OK) {
        ESP_LOGE(TAG, "WiFi init failed");
        Error_Handler();
    }

    if (!wifi_wait_connected(60000)) {
        ESP_LOGW(TAG, "WiFi timeout");
    }

    if (mqtt_init() == ESP_OK) {
        ESP_LOGI(TAG, "MQTT connected");
    } else {
        ESP_LOGW(TAG, "MQTT init failed");
    }

    g_data_mutex = xSemaphoreCreateMutex();
    if (g_data_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        Error_Handler();
    }

    BaseType_t ret;

    ret = xTaskCreatePinnedToCore(
        task_sensor_read, "SensorRead", 4096,
        NULL, 5, NULL, 0
    );
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create sensor task");
        Error_Handler();
    }

    ret = xTaskCreatePinnedToCore(
        task_process_publish, "ProcessPub", 4096,
        NULL, 3, NULL, 1
    );
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create process task");
        Error_Handler();
    }

    ESP_LOGI(TAG, "All tasks created");
}

void Error_Handler(void)
{
    while (1) { vTaskDelay(pdMS_TO_TICKS(100)); }
}