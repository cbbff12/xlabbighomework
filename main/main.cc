
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
//#include "gpio.h"//在stepper.h中包含
#include "uart.h"
#include "stepper.h"
#include "oled.h"

extern "C" void app_main(void)
{
    //初始化b
    //esp_task_wdt_deinit();
    /*
    led_init(GPIO_NUM_2);
    led_init(GPIO_NUM_13);
    led_init(GPIO_NUM_12);
    led_init(GPIO_NUM_14);
    led_init(GPIO_NUM_26);
    led_init(GPIO_NUM_27);
    led_init(GPIO_NUM_25);
    */
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
    led_config led_close = {
        .gpio_num = GPIO_NUM_39,
        .time = 0,
        .ledctrl = 0,
    };
    led_config led_use = {
        .gpio_num = GPIO_NUM_39,
        .time = 0,
        .ledctrl = 2,
    };
    led_config stepper_config1 = {//这里传参数复用了led_config结构体，.ledctrl，表示循环圈数,.time按照两位数分割分别表示4个GPIO引脚，。
        .gpio_num = GPIO_NUM_21,
        .time = 32332526,
        .ledctrl = 512,
    };
    led_config stepper_config2 = {//这里传参数复用了led_config结构体，.ledctrl，表示循环圈数,.time按照两位数分割分别表示4个GPIO引脚，。
        .gpio_num = GPIO_NUM_21,
        .time = 27141213,
        .ledctrl = 512,
    };
    led_config stepper_config3 = {//这里传参数复用了led_config结构体，.ledctrl，表示循环圈数,.time按照两位数分割分别表示4个GPIO引脚，。
        .gpio_num = GPIO_NUM_21,
        .time = 21191805,
        .ledctrl = 512,
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

    ESP_LOGI(TAG, "Hello World!");
    printf("Hello World from ESP32!\n");
    gpio_install_isr_service(0);

    xTaskCreate(led_task, "led_task", 2048, (void *)&led_open, 10, NULL);
    

    xTaskCreate(uart_echo_task, "uart_echo_task", 4096, NULL, 5, NULL);//串口任务

    xTaskCreate(video_player_task, "video_player", 8192, NULL, 5, NULL);//视频播放任务

    xTaskCreate(step_forward, "step_forward", 4096, (void *)&stepper_config1, 5, NULL);
    vTaskDelay(pdMS_TO_TICKS(10000));
    xTaskCreate(step_forward, "step_forward", 4096, (void *)&stepper_config2, 5, NULL);
    vTaskDelay(pdMS_TO_TICKS(10000));
    xTaskCreate(step_forward, "step_forward", 4096, (void *)&stepper_config3, 5, NULL);
    vTaskDelay(pdMS_TO_TICKS(10000));
    //xTaskCreate(step_backward, "step_backward", 4096, (void *)&stepper_config, 5, NULL);
    //xTaskCreate(oled_test_pattern, "video_player", 8192, NULL, 5, NULL);
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    
    }
}

