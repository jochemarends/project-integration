#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <type_traits>

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "rom/gpio.h"

namespace server {
    esp_err_t post_handler(httpd_req_t* req) {
        auto level = gpio_get_level(GPIO_NUM_12) ^ 1;
        gpio_set_level(GPIO_NUM_12, level);

        const char *resp_str = "POST request received";
        httpd_resp_send(req, resp_str, strlen(resp_str));

        return ESP_OK;
    }

    struct deleter {
        void operator()(httpd_handle_t server) {
            if (server) {
                httpd_stop(server);
            }
        }
    };

    using handle = std::unique_ptr<std::remove_pointer_t<httpd_handle_t>, deleter>;

    handle make() {
        static const httpd_uri_t uri_post{
            .uri      = "/toggle",
            .method   = HTTP_POST,
            .handler  = post_handler,
            .user_ctx = nullptr,
        };

        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        httpd_handle_t server = nullptr;
        if (httpd_start(&server, &config) == ESP_OK) {
            httpd_register_uri_handler(server, &uri_post);
        }

        return handle{server};
    }
}

namespace wifi {
    namespace {
        EventGroupHandle_t event_group{};
        constexpr auto connected_bit = BIT0;
        constexpr auto fail_bit = BIT1;
    }

    void event_handler(void* arg, esp_event_base_t event_base, std::int32_t event_id, void* event_data) {
        if (event_base == WIFI_EVENT) {
            if (event_id == WIFI_EVENT_STA_START) {
                esp_wifi_connect();
            }
            else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
                xEventGroupSetBits(event_group, fail_bit);
            }
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI("SERVER", "IP:" IPSTR, IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(event_group, connected_bit);
        }
    }

    void init_sta() {
        event_group = xEventGroupCreate();

        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        esp_event_handler_instance_t any_id{}, got_ip{};
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, nullptr, &any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, nullptr, &got_ip));

        wifi_config_t wifi_config{
            .sta{
                .ssid = "TP-LINK_3745",
                .password = "tilligte"
            },
        };

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        xEventGroupWaitBits(event_group, connected_bit | fail_bit, pdFALSE, pdFALSE, portMAX_DELAY);
    }
}

extern "C" void app_main() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    gpio_pad_select_gpio(GPIO_NUM_12);
    gpio_reset_pin(GPIO_NUM_12);
    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_INPUT_OUTPUT);

    wifi::init_sta();
    auto server = server::make();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }
}
