
#include "soc/gpio_num.h"
extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"            
#include "driver/ledc.h"  
#include "esp_log.h"          
}
#include "gpio.h"
#include "uart.h"

#include <nvs_flash.h>


extern "C" void app_main(void)
{

    led_init(GPIO_NUM_4);
    button_init(GPIO_NUM_36);
    button_init(GPIO_NUM_35);
    button_init(GPIO_NUM_34);
    button_init(GPIO_NUM_39);
    stepper_init();
    i2c_master_init();
    oled_init();
    oled_write_cmd(0xAE); // 关闭显示
    for(int i = 0; i < 8; i++) {
        oled_write_cmd(0xB0 + i);
        oled_write_cmd(0x00);
        oled_write_cmd(0x10);
        for(int j = 0; j < 128; j++) {
            oled_write_data((uint8_t*)"\x00", 1);
        }
    }
    oled_write_cmd(0xAF);

    led_config led_open = {
        .gpio_num = GPIO_NUM_39,
        .time = 0,
        .ledctrl = 1,
    };
    led_config GPIO36 = {
        .gpio_num = GPIO_NUM_36,
        .time = 0,
        .ledctrl = 0,
    };
    led_config GPIO34 = {
        .gpio_num = GPIO_NUM_34,
        .time = 0,
        .ledctrl = 0,
    };
    led_config GPIO35 = {
        .gpio_num = GPIO_NUM_35,
        .time = 0,
        .ledctrl = 0,
    };
    led_config led_use = {
        .gpio_num = GPIO_NUM_39,
        .time = 0,
        .ledctrl = 2,
    };

    uart_config_t uart_config = {
        .baud_rate = 115200,                    // 波特率
        .data_bits = UART_DATA_8_BITS,          // 8位数据
        .parity    = UART_PARITY_DISABLE,       // 无校验位
        .stop_bits = UART_STOP_BITS_1,          // 1位停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,  // 无硬件流控
        .source_clk = UART_SCLK_DEFAULT,        // 默认时钟源
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, 0, NULL, 0));
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_init_sta();
    
    


    ESP_LOGI(TAG, "Hello World!");
    printf("Hello World from ESP32!\n");
    oled_display_text("ESP32 Screen", "Multi-WiFi v1.0");
    gpio_install_isr_service(0);

    xTaskCreate(led_task, "led_task", 2048, (void *)&led_open, 10, NULL);
    xTaskCreate(screen_button_task, "oled_task", 2048, (void *)&GPIO36, 10, NULL);

    xTaskCreate(uart_echo_task, "uart_echo_task", 2048, NULL, 5, NULL);//串口任务

    //xTaskCreate(tcp_client_task, "tcp_client", 8192, NULL, 5, NULL);
    //xTaskCreate(video_player_task, "video_player", 8192, NULL, 5, NULL);//视频播放任务
    xTaskCreate(button_task1, "button_task1", 2048, &GPIO34, 5, NULL);//按钮任务
    xTaskCreate(button_task2, "button_task2", 2048, &GPIO35, 5, NULL);//按钮任务



    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    
    }
}

