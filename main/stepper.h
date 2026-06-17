#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"  // 用于微秒级延时
#include "init.h"
#include "esp_task_wdt.h"
#include "soc/gpio_num.h"
#include "esp_sntp.h"
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
/*
#define IN1_PIN GPIO_NUM_21
#define IN2_PIN GPIO_NUM_19
#define IN3_PIN GPIO_NUM_18
#define IN4_PIN GPIO_NUM_5*/
#define STEP_DELAY_US 2000
void led_init(gpio_num_t num)//设置为输出模式，函数名懒得改了
{
    // 配置GPIO为输出模式
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << num),  // 选择要配置的GPIO引脚
        .mode = GPIO_MODE_OUTPUT,            // 设置为输出模式
        .pull_up_en = GPIO_PULLUP_DISABLE,   // 启用内部上拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // 禁用下拉电阻
        .intr_type = GPIO_INTR_DISABLE       // 禁用中断
    };
    
    // 应用GPIO配置
    gpio_config(&io_conf);
    
    // 初始状态设置为低电平（LED关闭）
    gpio_set_level(num, 0);
}
void stepper_init(void){
    led_init(GPIO_NUM_5);
    led_init(GPIO_NUM_18);
    led_init(GPIO_NUM_19);
    led_init(GPIO_NUM_21);
    led_init(GPIO_NUM_32);
    led_init(GPIO_NUM_33);
    led_init(GPIO_NUM_25);
    led_init(GPIO_NUM_26);
    led_init(GPIO_NUM_27);
    led_init(GPIO_NUM_14);
    led_init(GPIO_NUM_12);
    led_init(GPIO_NUM_13);
}
gpio_num_t gpio_list[40]={GPIO_NUM_0,GPIO_NUM_1,GPIO_NUM_2,GPIO_NUM_3,GPIO_NUM_4,GPIO_NUM_5,GPIO_NUM_6,GPIO_NUM_7,GPIO_NUM_8,GPIO_NUM_9
,GPIO_NUM_10,GPIO_NUM_11,GPIO_NUM_12,GPIO_NUM_13,GPIO_NUM_14,GPIO_NUM_15,GPIO_NUM_16,GPIO_NUM_17,GPIO_NUM_18,GPIO_NUM_19
,GPIO_NUM_20,GPIO_NUM_21,GPIO_NUM_22,GPIO_NUM_23,GPIO_NUM_0,GPIO_NUM_25,GPIO_NUM_26,GPIO_NUM_27,GPIO_NUM_28,GPIO_NUM_29
,GPIO_NUM_30,GPIO_NUM_31,GPIO_NUM_32,GPIO_NUM_33,GPIO_NUM_34,GPIO_NUM_35,GPIO_NUM_36,GPIO_NUM_37,GPIO_NUM_38,GPIO_NUM_39
};
void step_forward(void *pvParameters) {
    led_config *led1_config = (led_config *)pvParameters;
    gpio_num_t gpio_num = led1_config->gpio_num;
    int steps = led1_config->ledctrl;
    gpio_num_t IN1_PIN = gpio_list[led1_config->time%100];
    gpio_num_t IN2_PIN = gpio_list[led1_config->time/100%100];
    gpio_num_t IN3_PIN = gpio_list[led1_config->time/10000%100];
    gpio_num_t IN4_PIN = gpio_list[led1_config->time/1000000];
    ESP_LOGI(TAG, "step_forward_start");
    for(int i = 0; i < steps; i++){

    gpio_set_level(IN1_PIN, 1); gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 0); gpio_set_level(IN4_PIN, 0);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 1); gpio_set_level(IN2_PIN, 1);
    gpio_set_level(IN3_PIN, 0); gpio_set_level(IN4_PIN, 0);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 0); gpio_set_level(IN2_PIN, 1);
    gpio_set_level(IN3_PIN, 0); gpio_set_level(IN4_PIN, 0);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 0); gpio_set_level(IN2_PIN, 1);
    gpio_set_level(IN3_PIN, 1); gpio_set_level(IN4_PIN, 0);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 0); gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 1); gpio_set_level(IN4_PIN, 0);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 0); gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 1); gpio_set_level(IN4_PIN, 1);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 0); gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 0); gpio_set_level(IN4_PIN, 1);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 1); gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 0); gpio_set_level(IN4_PIN, 1);
    esp_rom_delay_us(STEP_DELAY_US);

    taskYIELD();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_LOGI(TAG, "step_forward_end");
    vTaskDelete(NULL);

}
void step_backward(void *pvParameters) {
    led_config *led1_config = (led_config *)pvParameters;
    gpio_num_t gpio_num = led1_config->gpio_num;
    int steps = led1_config->ledctrl;
    gpio_num_t IN1_PIN = gpio_list[led1_config->time%100];
    gpio_num_t IN2_PIN = gpio_list[led1_config->time/100%100];
    gpio_num_t IN3_PIN = gpio_list[led1_config->time/10000%100];
    gpio_num_t IN4_PIN = gpio_list[led1_config->time/1000000];
    int i = 0;
    ESP_LOGI(TAG, "step_backward_start");
    for(i = 0; i < steps; i++){
        
    gpio_set_level(IN1_PIN, 1); gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 0); gpio_set_level(IN4_PIN, 1);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 0); gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 0); gpio_set_level(IN4_PIN, 1);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 0); gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 1); gpio_set_level(IN4_PIN, 1);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 0); gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 1); gpio_set_level(IN4_PIN, 0);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 0); gpio_set_level(IN2_PIN, 1);
    gpio_set_level(IN3_PIN, 1); gpio_set_level(IN4_PIN, 0);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 0); gpio_set_level(IN2_PIN, 1);
    gpio_set_level(IN3_PIN, 0); gpio_set_level(IN4_PIN, 0);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 1); gpio_set_level(IN2_PIN, 1);
    gpio_set_level(IN3_PIN, 0); gpio_set_level(IN4_PIN, 0);
    esp_rom_delay_us(STEP_DELAY_US);
    

    gpio_set_level(IN1_PIN, 1); gpio_set_level(IN2_PIN, 0);
    gpio_set_level(IN3_PIN, 0); gpio_set_level(IN4_PIN, 0);
    esp_rom_delay_us(STEP_DELAY_US);

    taskYIELD();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_LOGI(TAG, "step_backward_end");
    vTaskDelete(NULL);
}
int lasthour = 0;
int lastminute =  0;
void step_time(void *pvParameters){
    led_config stepper_config[6]={{.gpio_num = GPIO_NUM_21,
        .time = 32332526,
        .ledctrl = 43,},{.gpio_num = GPIO_NUM_21,
        .time = 27141213,
        .ledctrl = 43,},{.gpio_num = GPIO_NUM_21,
        .time = 21191805,
        .ledctrl = 43,},{.gpio_num = GPIO_NUM_21,
        .time = 32332526,
        .ledctrl = 42,},{.gpio_num = GPIO_NUM_21,
        .time = 27141213,
        .ledctrl = 297,},{.gpio_num = GPIO_NUM_21,
        .time = 21191805,
        .ledctrl = 125,}};
    while(1){
        time_t now;
    struct tm timeinfo;

    // 1. 获取当前时间戳（秒）
    time(&now);

    // 2. 转换为本地时间结构体
    //    注意：请确保之前已经调用过 setenv("TZ", "CST-8", 1) 和 tzset() 设置了时区
    localtime_r(&now, &timeinfo);

    // 3. 直接读取 int 类型的小时和分钟
    int hour = timeinfo.tm_hour;   // 范围 0-23
    int minute = timeinfo.tm_min;  // 范围 0-59
        if(hour != lasthour){
            lasthour = hour;
            lastminute = minute;
            //oled_display_text("当前时间", " %d点%d分", hour, minute);
            if(hour%3){
                xTaskCreate(step_forward, "step_forward1", 4096, (void *)&stepper_config[0], 5, NULL);
            }else{
                xTaskCreate(step_forward, "step_forward2", 4096, (void *)&stepper_config[3], 5, NULL);
            }
        }
        if(minute != lastminute){
            lastminute = minute;
           // oled_display_text("当前时间", " %d点%d分", hour, minute);
            if(minute%10==0){
                xTaskCreate(step_forward, "step_forward3", 4096, (void *)&stepper_config[5], 5, NULL);
            }else{
                xTaskCreate(step_forward, "step_forward4", 4096, (void *)&stepper_config[2], 5, NULL);
            }
            if(minute == 0){
                xTaskCreate(step_forward, "step_forward5", 4096, (void *)&stepper_config[4], 5, NULL);
                
            }else if(minute % 10 ==0){
                xTaskCreate(step_forward, "step_forward6", 4096, (void *)&stepper_config[1], 5, NULL);
            }
                       
        }
    //ESP_LOGI(TAG, "当前时间: %d点%d分", hour, minute);
    vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
