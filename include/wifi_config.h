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

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

#define PROV_SERVICE_NAME   "SMART_BED_PROV"
#define PROV_POP            "123456"

#define MAX_RETRY_COUNT 5

extern EventGroupHandle_t s_wifi_event_group;

esp_err_t wifi_init_sta_with_provisioning(void);
bool wifi_is_connected(void);
bool wifi_wait_connected(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif