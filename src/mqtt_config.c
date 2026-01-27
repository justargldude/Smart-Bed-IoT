/**
 * @file    mqtt_config.c
 * @brief   MQTT client configuration and initialization for ESP32
 *
 * @details This file implements MQTT client setup, connection, and event handling
 *          for secure communication with a cloud MQTT broker (e.g., HiveMQ).
 *          It provides initialization, start/stop, and event callback logic.
 **/

#include "mqtt_config.h"

// Tag used for ESP-IDF logging
static const char *TAG = "MQTT";

// Global MQTT client handle
static esp_mqtt_client_handle_t mqtt_client = NULL;

/**
 * @brief   MQTT event handler callback
 *
 * @param   handler_args   User-defined handler arguments (unused)
 * @param   base           Event base (unused)
 * @param   event_id       Event ID (MQTT event type)
 * @param   event_data     Pointer to event data (esp_mqtt_event_handle_t)
 *
 * @details Handles connection, subscription, data reception, errors, and publish events.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) 
{
    // Cast event data to MQTT event handle
    esp_mqtt_event_handle_t event = event_data; 
    static int32_t subscribe_msg_id = -1;

    switch (event->event_id)
    {
        case MQTT_EVENT_CONNECTED:
            // Connected to MQTT broker, subscribe to data topic
            subscribe_msg_id  = esp_mqtt_client_subscribe(mqtt_client, TOPIC_SUB_CMD, 1);
            if (subscribe_msg_id  >= 0) {
                ESP_LOGI(TAG, "Subscribe sent (id=%d)", subscribe_msg_id );
            } else {
                ESP_LOGE(TAG, "Subscribe failed (err=%d)", subscribe_msg_id );
            }
            break;

        case MQTT_EVENT_SUBSCRIBED: {
            // Subscription acknowledged by broker
            if (event->msg_id == subscribe_msg_id ) {
                ESP_LOGI(TAG, "Subscription confirmed");
                subscribe_msg_id  = -1; // Reset message ID
            }
            break;
        }
        case MQTT_EVENT_DISCONNECTED:
            // Disconnected from MQTT broker
            ESP_LOGW(TAG, "DISCONNECTED");
            break;

        case MQTT_EVENT_DATA: {
            // Data received from broker
            char cmd[STRING_LEN];
            ESP_LOGI(TAG, "[%.*s] %.*s", event->topic_len, event->topic, event->data_len, event->data);
            uint8_t len = (event->data_len < STRING_LEN - 1) ? event->data_len : STRING_LEN - 1;
            memcpy(cmd, event->data, len);
            cmd[len] = '\0';
            if (strcmp(cmd, "RESET_FALL") == 0) {
                ESP_LOGW(TAG, "Reset Fall Alert");
            } 
            else if (strcmp(cmd, "GET_STATUS") == 0) {
                ESP_LOGI(TAG, "Sending device status...");
            }
            else {
                ESP_LOGW(TAG, "Unknown command: %s", cmd);
            }
            break;
        }

        case MQTT_EVENT_ERROR:
            // Error occurred in MQTT communication
            ESP_LOGW(TAG, "MQTT ERROR type = %d", event->error_handle->error_type);
            break;

        case MQTT_EVENT_PUBLISHED:
            // Publish operation confirmed by broker
            ESP_LOGI(TAG, "Publish confirmed (msg_id=%d)", event->msg_id);
            break;

        default:
            // Handle all other events
            ESP_LOGD(TAG, "Unhandled event: %d", event->event_id);
            break;
    }
}

/**
 * @brief   Initialize and start the MQTT client
 *
 * @return  ESP_OK on success, ESP_FAIL on error
 */
esp_err_t mqtt_init(void)
{

    // MQTT client configuration structure
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .broker.address.port = MQTT_BROKER_PORT,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
        .broker.verification.certificate = (const char *)root_ca_pem_start,
    };

    // Initialize MQTT client
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to init MQTT client");
        return ESP_FAIL;
    }

    // Register event handler for all MQTT events
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    // Start MQTT client
    esp_mqtt_client_start(mqtt_client);
    return ESP_OK;
}

/**
 * @brief   Start the MQTT client if already initialized
 *
 * @return  ESP_OK on success, ESP_FAIL if not initialized
 */
esp_err_t mqtt_start(void)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return ESP_FAIL;
    }
    return esp_mqtt_client_start(mqtt_client);
}

/**
 * @brief   Stop the MQTT client
 *
 * @return  ESP_OK on success, ESP_FAIL if not initialized
 */
esp_err_t mqtt_stop(void)
{
    if (mqtt_client == NULL) {
        return ESP_FAIL;
    }
    return esp_mqtt_client_stop(mqtt_client);
}