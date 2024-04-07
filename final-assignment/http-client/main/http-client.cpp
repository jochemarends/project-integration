#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "rom/gpio.h"

#define TAG "HTTP CLIENT"

int id = 1;

namespace http {
    esp_err_t event_handler(esp_http_client_event_t* evt) {
        switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        default:
            break;
        }
        return ESP_OK;
    }
}

void log() {
    esp_http_client_config_t config{
        .url = "http://204.168.244.220:8080/log-event?id=1&msg=Button%20Pressed",
    };

    auto client = esp_http_client_init(&config);
    if (auto err = esp_http_client_perform(client); err == ESP_OK) {
        ESP_LOGI(TAG, "LOGGED!");
    }
}

namespace wifi {
    EventGroupHandle_t event_group{};

    namespace bits {
        constexpr int connected = BIT0;
        constexpr int fail = BIT1;
    }

    void event_handler(void* arg, esp_event_base_t event_base, std::int32_t event_id, void* event_data) {
        if (event_base == WIFI_EVENT) {
            if (event_id == WIFI_EVENT_STA_START) {
                esp_wifi_connect();
            }
            else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
                xEventGroupSetBits(event_group, bits::fail);
            }
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            auto event = reinterpret_cast<ip_event_got_ip_t*>(event_data);
            ESP_LOGI("WIFI", "IP:" IPSTR, IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(event_group, bits::connected);
        }
    }

    void init_sta() {
        event_group = xEventGroupCreate();
        
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        // register handlers for WiFi and IP events
        esp_event_handler_instance_t any_id{};
        esp_event_handler_instance_t got_ip{};
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, nullptr, &any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, nullptr, &got_ip));

        // WiFi configuration with credentials
        wifi_config_t wifi_config{
            .sta{
                .ssid = "TP-LINK_3745",
                .password = "tilligte",
            },
        };

        // start WiFi
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        // wait for one of the events to occur
        xEventGroupWaitBits(event_group, bits::connected | bits::fail, pdFALSE, pdFALSE, portMAX_DELAY);
    }
}

extern "C" void app_main() {
    // try to initialize non-volatile storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // initialize WiFi using station mode
    wifi::init_sta();

    esp_http_client_config_t config{
        .url = "http://204.168.244.220:8080/add-patient?name=Jochem",
        .event_handler = http::event_handler,
    };

    auto client = esp_http_client_init(&config);
    if (auto err = esp_http_client_perform(client); err == ESP_OK) {
        // something
    }

    esp_http_client_cleanup(client);

    constexpr gpio_num_t button_pin = GPIO_NUM_27;
    int prev_level{};

    gpio_pad_select_gpio(button_pin);
    gpio_reset_pin(button_pin);
    gpio_set_direction(button_pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(button_pin, GPIO_PULLDOWN_ONLY); 

    while (true) {
        auto curr_level = gpio_get_level(button_pin);
        if (prev_level == 0 && curr_level == 1) {
            //ESP_LOGI(TAG, "HOI");
            log();
        }
        prev_level = curr_level;
        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}

