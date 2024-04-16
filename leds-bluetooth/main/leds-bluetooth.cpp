#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iostream>
#include <thread>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define GATTS_TAG "GATTS"
#define DEVICE_NAME "ESP32-JA"

#define APP_ID 0

namespace ble {
    std::uint8_t adv_service_uuid128[32]{
        0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
        0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
    };

    esp_ble_adv_data_t adv_data{
        .set_scan_rsp = false,
        .include_name = true,
        .include_txpower = false,
        .min_interval = 0x0006,
        .max_interval = 0x0010,
        .appearance = 0x00,
        .manufacturer_len = 0,
        .p_manufacturer_data =  NULL,
        .service_data_len = 0,
        .p_service_data = NULL,
        .service_uuid_len = sizeof(adv_service_uuid128),
        .p_service_uuid = adv_service_uuid128,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
    };

    esp_ble_adv_data_t scan_rsp_data{
        .set_scan_rsp = true,
        .include_name = true,
        .include_txpower = true,
        .appearance = 0x00,
        .manufacturer_len = 0,
        .p_manufacturer_data =  NULL,
        .service_data_len = 0,
        .p_service_data = NULL,
        .service_uuid_len = sizeof(adv_service_uuid128),
        .p_service_uuid = adv_service_uuid128,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
    };

    esp_ble_adv_params_t adv_params{
        .adv_int_min = 0x20,
        .adv_int_max = 0x40,
        .adv_type = ADV_TYPE_IND,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };

    std::uint8_t adv_config_done{};
    constexpr std::uint8_t adv_config_flag{1 << 0};
    constexpr std::uint8_t scan_rsp_config_flag{1 << 1};

    void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
        switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~adv_config_flag);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~scan_rsp_config_flag);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(GATTS_TAG, "Advertising start failed");
            }
            else {
                ESP_LOGE(GATTS_TAG, "Advertising start successfully");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(GATTS_TAG, "Advertising stop failed");
            } 
            else {
                ESP_LOGI(GATTS_TAG, "Stop adv successfully");
            }
            break;
        default:
            break;
        }
    }

    esp_gatt_srvc_id_t service_id{};
    std::uint16_t service_handle{};
    std::uint16_t char_handle{};
    std::uint16_t descr_handle{};

    esp_bt_uuid_t char_uuid;

    constexpr std::uint16_t uuid = 0x00EE;
    constexpr std::uint16_t char_uuid_num = 0xEE01;
    constexpr std::uint16_t num_handle = 4;
    esp_gatt_char_prop_t b_property{};

    void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param) {
        switch (event) {
        case ESP_GATTS_REG_EVT: {
            service_id.is_primary = true;
            service_id.id.inst_id = 0x00;
            service_id.id.uuid.len = ESP_UUID_LEN_16;
            service_id.id.uuid.uuid.uuid16 = uuid;
            esp_ble_gap_set_device_name("ESP-32-LEDS-BLE");
            auto ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret){
                ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
            }
            adv_config_done |= adv_config_flag;

            //config scan response data
            ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            if (ret){
                ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
            }
            adv_config_done |= scan_rsp_config_flag;
            esp_ble_gatts_create_service(gatts_if, &service_id, num_handle);
            break;
        }
        case ESP_GATTS_CREATE_EVT: {
            service_handle = param->create.service_handle;
            char_uuid.len = ESP_UUID_LEN_16;
            char_uuid.uuid.uuid16 = char_uuid_num;
            esp_ble_gatts_start_service(service_handle);
            b_property = ESP_GATT_CHAR_PROP_BIT_WRITE;
            esp_ble_gatts_add_char(service_handle, &char_uuid, ESP_GATT_PERM_WRITE, b_property, NULL, NULL);
            break;
        }
        case ESP_GATTS_CONNECT_EVT: {
             esp_ble_conn_update_params_t conn_params = {0};
             memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
             conn_params.latency = 0;
             conn_params.max_int = 0x30;
             conn_params.min_int = 0x10;
             conn_params.timeout = 400;
             // gl_profile_tab[PROFILE_A_APP_ID].conn_id = param->connect.conn_id;
             esp_ble_gap_update_conn_params(&conn_params);
         break;
        }
        case ESP_GATTS_WRITE_EVT:
        default:
            break;
        }
    }
}


extern "C" void app_main() {
    auto ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // configure bluetooth controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    // configure bluedroid
    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bluedroid_init_with_cfg(&bluedroid_cfg));
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    // register callbacks
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(ble::gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(ble::gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(APP_ID));

    ESP_ERROR_CHECK(esp_ble_gatt_set_local_mtu(512));

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }
}
