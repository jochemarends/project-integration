#include <stdio.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#define WIFI_SSID "TP-LINK_4935"
#define WIFI_PASSWORD "tilligte"

#define WIFI_CONNECTED_BIT BTI0
#define WIFI_FAIL_BIT      BTI1

EventGroupHandle_t wifi_event_group{};

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECT) {
            
        }
    }
}

void wifi_init_sta() {
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };

}

extern "C" void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
}

