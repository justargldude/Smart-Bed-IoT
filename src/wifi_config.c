/**
 * @file wifi_config.c
 * @brief WiFi + BLE Provisioning for ESP32-S3 (NimBLE)
 */

#include "wifi_config.h"

static const char *TAG = "WIFI";

EventGroupHandle_t s_wifi_event_group = NULL;

static int s_retry_num = 0;

// Event handler for WiFi, IP, and Provisioning events
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // WiFi events
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                if (s_wifi_event_group) {
                    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                }
                if (s_retry_num < MAX_RETRY_COUNT) {
                    s_retry_num++;
                    ESP_LOGW(TAG, "Retry %d/%d", s_retry_num, MAX_RETRY_COUNT);
                    esp_wifi_connect();
                } else {
                    ESP_LOGE(TAG, "Connect failed");
                    if (s_wifi_event_group) {
                        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                    }
                }
                break;

            default:
                break;
        }
    }
    // IP events
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected, IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        if (s_wifi_event_group) {
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }
    // Provisioning events
    else if (event_base == WIFI_PROV_EVENT)
    {
        switch (event_id)
        {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Prov started: %s (PoP: %s)", PROV_SERVICE_NAME, PROV_POP);
                break;

            case WIFI_PROV_CRED_RECV:
            {
                wifi_sta_config_t *cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG, "Got SSID: %s", (char *)cfg->ssid);
                break;
            }

            case WIFI_PROV_CRED_FAIL:
            {
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE(TAG, "Prov failed: %s", 
                        (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Auth error" : "AP not found");
                break;
            }

            case WIFI_PROV_END:
                wifi_prov_mgr_deinit();
                break;

            default:
                break;
        }
    }
}

esp_err_t wifi_init_sta_with_provisioning(void)
{
    esp_err_t ret;

    // Init NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Create event group
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Event group create failed");
        return ESP_FAIL;
    }

    // Init netif and event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // Init WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    // Init provisioning manager
    wifi_prov_mgr_config_t prov_config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(prov_config));

    // Check provisioning status
    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (provisioned) {
        ESP_LOGI(TAG, "Already provisioned");
        wifi_prov_mgr_deinit();
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    } else {
        ESP_LOGI(TAG, "Starting BLE provisioning");
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(
            WIFI_PROV_SECURITY_1, PROV_POP, PROV_SERVICE_NAME, NULL));
    }

    return ESP_OK;
}

bool wifi_is_connected(void)
{
    if (s_wifi_event_group == NULL) return false;
    return (xEventGroupGetBits(s_wifi_event_group) & WIFI_CONNECTED_BIT) != 0;
}

bool wifi_wait_connected(uint32_t timeout_ms)
{
    if (s_wifi_event_group == NULL) return false;

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(timeout_ms)
    );

    return (bits & WIFI_CONNECTED_BIT) != 0;
}