#ifndef WIFI_H
#define WIFI_H

#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_COUNT 3
#define MAX_RETRY_COUNT 3
#define RETRY_DELAY_MS 5000
#define WIFI_SWITCH_DELAY_MS 10000

extern EventGroupHandle_t wifi_event_group;
extern const int WIFI_CONNECTED_BIT;

// WiFi配置结构体
typedef struct {
    char ssid[32];
    char password[64];
} wifi_config_item_t;

// 函数声明
esp_err_t wifi_init_sta(void);
void switch_to_next_wifi(void);
void wifi_event_handler(void* arg, esp_event_base_t event_base,
                        int32_t event_id, void* event_data);

#ifdef __cplusplus
}
#endif

#endif // WIFI_H