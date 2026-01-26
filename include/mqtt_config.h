#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#include "esp_err.h"
#include "mqtt_client.h"
#include "esp_log.h"

// MQTT Broker Configuration
#define MQTT_BROKER_URI      "mqtts://210b526e980b475fb491288bd347d468.s1.eu.hivemq.cloud"
#define MQTT_BROKER_PORT     8883
#define MQTT_USERNAME        "esp32_device"
#define MQTT_PASSWORD        "DevicePass123"

#define TOPIC_PUB_DATA       "smartcrib/data"
#define TOPIC_SUB_CMD        "smartcrib/cmd"

#define STRING_LEN 128

extern const uint8_t root_ca_pem_start[] asm("_binary_root_ca_pem_start");
extern const uint8_t root_ca_pem_end[]   asm("_binary_root_ca_pem_end");

// Function prototypes
esp_err_t mqtt_init(void);
esp_err_t mqtt_start(void);
esp_err_t mqtt_stop(void);

#endif
