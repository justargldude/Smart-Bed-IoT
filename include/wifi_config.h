/**
 * @file wifi_config.h
 * @brief WiFi + BLE Provisioning for ESP32-S3 (NimBLE)
 */

#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"
#include "esp_netif.h"

// Event bits for WiFi state
#define WIFI_CONNECTED_BIT  BIT0  // Connected and got IP
#define WIFI_FAIL_BIT       BIT1  // Connection failed

// BLE Provisioning config
#define PROV_SERVICE_NAME   "SMART_BED_PROV"  // BLE device name
#define PROV_POP            "123456"          // Proof of Possession

#define MAX_RETRY_COUNT 5

// Event group for WiFi state sync between tasks
extern EventGroupHandle_t s_wifi_event_group;

// Init WiFi with BLE Provisioning (auto-connect if already provisioned)
esp_err_t wifi_init_sta_with_provisioning(void);

// Check WiFi connection status (non-blocking)
bool wifi_is_connected(void);

// Wait for WiFi connection with timeout (blocking)
bool wifi_wait_connected(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_CONFIG_H */