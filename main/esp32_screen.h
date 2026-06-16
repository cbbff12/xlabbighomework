extern "C"
{
#ifndef ESP32_SCREEN_H
#define ESP32_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

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
#include <driver/i2c.h>
#include <esp_timer.h>
#include "wifi.h"
#include "init.h"

// OLED配置
#define OLED_I2C_PORT I2C_NUM_0
#define OLED_I2C_SDA_PIN GPIO_NUM_23
#define OLED_I2C_SCL_PIN GPIO_NUM_22
#define OLED_I2C_FREQ_HZ 400000
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_PAGES (SCREEN_HEIGHT / 8)
#define IMAGE_BUFFER_SIZE (SCREEN_WIDTH * SCREEN_PAGES)
#define TASK_STOP_BIT (1 << 2)

// 服务器配置
#define SERVER_HOST "www.fatsilkworm.top"
#define SERVER_PORT 20500
#define AUTH_PASSWORD ""

// 函数声明
void oled_init2(void);
void oled_display_image(uint8_t *buffer);
void oled_display_text(const char* line1, const char* line2);
void tcp_client_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // ESP32_SCREEN_H
}
