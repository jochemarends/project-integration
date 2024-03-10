#include <cstdint>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "nvs_flash.h"

#define PROFILE_A_APP_ID 0

struct T{
esp_gatts_cb_t gatts_cb;
uint16_t gatts_if;
uint16_t app_id;
uint16_t conn_id;
uint16_t service_handle;
uint16_t char_handle;
esp_bt_uuid_t char_uuid;
esp_gatt_perm_t perm;
esp_gatt_char_prop_t property;
uint16_t descr_handle;
esp_bt_uuid_t descr_uuid;
};

esp_gatt_srvc_id_t service_id{};
std::uint16_t service_handle{};
std::uint16_t char_handle{};
std::uint16_t descr_handle{};

constexpr std::uint16_t uuid = 0x00EE;
constexpr std::uint16_t num_handle = 4;

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param) {
    switch (event) {
    case ESP_GATTS_REG_EVT:
        service_id.is_primary = true;
        service_id.id.inst_id = 0x00;
        service_id.id.uuid.len = ESP_UUID_LEN_16;
        service_id.id.uuid.uuid.uuid16 = uuid;
        esp_ble_gatts_create_service(gatts_if, &service_id, num_handle);
    case ESP_GATTS_WRITE_EVT:
        service_handle = param->create.service_handle;
    }
}

extern "C" void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());

    // configure bluetooth controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    // configure bluedroid
    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bluedroid_init_with_cfg(&bluedroid_cfg));
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    // register callbacks
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
//    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(PROFILE_A_APP_ID));
}
