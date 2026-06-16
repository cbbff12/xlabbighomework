extern "C"
{
#include "wifi.h"
#include "init.h"  // 包含 TAG 定义



// WiFi列表 - 定义在.c文件中
static const wifi_config_item_t wifi_list[] = {
    {"dixiashi", "0er3456789"},
    {"cbbffsj", "acti5588"},
    {"YOUR_WIFI_SSID_3", "YOUR_WIFI_PASSWORD_3"},
};

// 全局变量定义
EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

static int current_wifi_index = 0;
static int retry_count = 0;

void switch_to_next_wifi(void) {
    current_wifi_index = (current_wifi_index + 1) % WIFI_COUNT;
    retry_count = 0;
    
    ESP_LOGI(TAG, "切换到WiFi %d/%d: %s", 
             current_wifi_index + 1, WIFI_COUNT, 
             wifi_list[current_wifi_index].ssid);
    
    wifi_config_t wifi_config = {
        .sta = {
            .threshold = {.authmode = WIFI_AUTH_WPA2_PSK},
        },
    };
    
    strcpy((char*)wifi_config.sta.ssid, wifi_list[current_wifi_index].ssid);
    strcpy((char*)wifi_config.sta.password, wifi_list[current_wifi_index].password);
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_wifi_connect();
}

void wifi_event_handler(void* arg, esp_event_base_t event_base,
                        int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "尝试连接WiFi: %s", wifi_list[current_wifi_index].ssid);
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi断开连接: %s", wifi_list[current_wifi_index].ssid);
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        
        retry_count++;
        
        if (retry_count < MAX_RETRY_COUNT) {
            ESP_LOGI(TAG, "重试当前WiFi (%d/%d)", retry_count, MAX_RETRY_COUNT);
            esp_wifi_connect();
        } else {
            ESP_LOGW(TAG, "WiFi %s 连接失败，尝试下一个", wifi_list[current_wifi_index].ssid);
            switch_to_next_wifi();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "WiFi连接成功！SSID: %s, IP: " IPSTR, 
                 wifi_list[current_wifi_index].ssid,
                 IP2STR(&event->ip_info.ip));
        
        retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    
    wifi_config_t wifi_config = {
        .sta = {
            .threshold = {.authmode = WIFI_AUTH_WPA2_PSK},
        },
    };
    
    strcpy((char*)wifi_config.sta.ssid, wifi_list[0].ssid);
    strcpy((char*)wifi_config.sta.password, wifi_list[0].password);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi初始化完成，将尝试连接 %d 个网络", WIFI_COUNT);
    return ESP_OK;
}
}