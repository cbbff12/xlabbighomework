#include "hal/gpio_types.h"
#include "soc/gpio_num.h"
extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"    
#include "esp_log.h"    
#include <string.h>                  
#include <nvs_flash.h>
#include "esp_sntp.h"
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
}
#include "init.h"
#include "esp32_screen.h"
#include "oled.h"
#include "stepper.h"


TaskHandle_t Task1 = NULL;
TaskHandle_t Task2 = NULL;


#define LED_GPIO GPIO_NUM_2
#define DEBOUNCE_DELAY_MS  101// 按钮消抖延时（毫秒）

static int led_state = 0;
static int oled_state = 0;
static bool last_button_state = true; 



void button_init(gpio_num_t num)//设置为输入模式
{
    // 配置GPIO为输入模式
    gpio_config_t io_conf_in = {
        .pin_bit_mask = (1ULL << num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,   // 使能上拉电阻
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE      // 不使用中断，采用轮询方式
    };
    gpio_config(&io_conf_in);
}
void IRAM_ATTR button_isr_handler(void *pvParameters)
{
    led_config *led1_config = (led_config *)pvParameters;
    int time = led1_config->time;
    gpio_num_t gpio_num = led1_config->gpio_num;
    led_state = (led1_config->ledctrl)?(((led1_config->ledctrl)==2)?led_state:1):0;
}

static bool last_button_state2 = true; 
void screen_button_task(void *pvParameters)
{
    led_config *led1_config = (led_config *)pvParameters;
    gpio_num_t gpio_num = led1_config->gpio_num;
    int time = led1_config->time;
    int ledctrl = led1_config->ledctrl;
    bool current_button_state;
    uint32_t last_interrupt_time = 0;
    xTaskCreate(video_player_task, "video_player", 8192, NULL, 5, &Task2);
    while (1) {
        current_button_state = gpio_get_level(gpio_num);
        //printf("%d\n", current_button_state);
        // 检测下降沿（从高电平变为低电平，即按钮按下）
        if (current_button_state == 0 && last_button_state2 == 1) {
            // 简单的软件消抖
            uint32_t current_time = xTaskGetTickCount();
            if ((current_time - last_interrupt_time) > pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)) {
                ESP_LOGI(TAG, "button 4");
                // 有效按下，切换LED状态
                oled_state = !oled_state;
                if(oled_state){
                    vTaskDelete(Task2);
                    xEventGroupClearBits(wifi_event_group, TASK_STOP_BIT);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    xTaskCreate(tcp_client_task, "tcp_client", 8192, NULL, 5, &Task1);

                }else{
                    xEventGroupSetBits(wifi_event_group, TASK_STOP_BIT);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    //vTaskDelete(Task1);
                    xTaskCreate(video_player_task, "video_player", 8192, NULL, 5, &Task2);//视频播放任务
                }
                printf("Button pressed! 2");
                
                last_interrupt_time = current_time;
            }
        }
        
        last_button_state2 = current_button_state;
        
        // 每30ms扫描一次
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static bool last_button_state3 = true; 
static int circle[3] = {0,0,0},stepper_num = 0; 

void led_task(void *pvParameters)
{
    led_config *led1_config = (led_config *)pvParameters;
    gpio_num_t gpio_num = led1_config->gpio_num;
    int time = led1_config->time;
    int ledctrl = led1_config->ledctrl;
    bool current_button_state;
    uint32_t last_interrupt_time = 0;
    ESP_LOGI(TAG, "select stepper%d",1);
    while (1) {
        // 读取当前按钮状态（GPIO35上拉，按下为低电平）
        current_button_state = gpio_get_level(gpio_num);
        //printf("%d\n", current_button_state);
        // 检测下降沿（从高电平变为低电平，即按钮按下）
        if (current_button_state == 0 && last_button_state == 1) {
            // 简单的软件消抖
            uint32_t current_time = xTaskGetTickCount();
            if ((current_time - last_interrupt_time) > pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)) {
                ESP_LOGI(TAG, "button 3");
                stepper_num=(stepper_num+1)%3;
                ESP_LOGI(TAG, "select stepper%d",stepper_num+1);
                //printf("Button pressed! 3");
                
                last_interrupt_time = current_time;
            }
        }
        
        last_button_state = current_button_state;
        
        // 每10ms扫描一次
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


    
void button_task1(void *pvParameters)
{
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
        .ledctrl = 42,},{.gpio_num = GPIO_NUM_21,
        .time = 21191805,
        .ledctrl = 42,}};
    led_config *led1_config = (led_config *)pvParameters;
    gpio_num_t gpio_num = led1_config->gpio_num;
    int time = led1_config->time;
    int ledctrl = led1_config->ledctrl;
    bool current_button_state;
    uint32_t last_interrupt_time = 0;
    while (1) {
        current_button_state = gpio_get_level(gpio_num);
        //printf("%d\n", current_button_state);
        // 检测下降沿（从高电平变为低电平，即按钮按下）
        if (current_button_state == 0 && last_button_state3 == 1) {
            // 简单的软件消抖
            uint32_t current_time = xTaskGetTickCount();
            if ((current_time - last_interrupt_time) > pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)) {
                ESP_LOGI(TAG, "button 2");
                // 有效按下，切换LED状态
                circle[stepper_num]++;
                //if(stepper_num<1||circle[stepper_num]<10){
                    
                
                if(circle[stepper_num]%3==0){
                    xTaskCreate(step_forward, "step_forward", 4096, (void *)&stepper_config[stepper_num], 5, NULL);
                vTaskDelay(pdMS_TO_TICKS(1000));
                ESP_LOGI(TAG, "stepper%d move 43",stepper_num+1);
                }else{
                    xTaskCreate(step_forward, "step_forward", 4096, (void *)&stepper_config[stepper_num+3], 5, NULL);
                vTaskDelay(pdMS_TO_TICKS(1000));
                ESP_LOGI(TAG, "stepper%d move 42",stepper_num+1);
                }
                /*}else{
                    if(stepper_num==1){
                        xTaskCreate(step_forward, "step_forward", 4096, (void *)&stepper_config2, 5, NULL);
                        vTaskDelay(pdMS_TO_TICKS(1000));
                    }else if(stepper_num==2){
                        xTaskCreate(step_forward, "step_forward", 4096, (void *)&stepper_config3, 5, NULL);
                        vTaskDelay(pdMS_TO_TICKS(1000));
                    }

                }*/
                
            }
        }
        
        last_button_state3 = current_button_state;
        
        // 每10ms扫描一次
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
static bool last_button_state4 = true; 
    
void button_task2(void *pvParameters)
{
    
    led_config *led1_config = (led_config *)pvParameters;
    gpio_num_t gpio_num = led1_config->gpio_num;
    int time = led1_config->time;
    int ledctrl = led1_config->ledctrl;
    bool current_button_state;
    uint32_t last_interrupt_time = 0;
    while (1) {
        current_button_state = gpio_get_level(gpio_num);
        //printf("%d\n", current_button_state);
        // 检测下降沿（从高电平变为低电平，即按钮按下）
        if (current_button_state == 0 && last_button_state4 == 1) {
            // 简单的软件消抖
            uint32_t current_time = xTaskGetTickCount();
            if ((current_time - last_interrupt_time) > pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)) {
                ESP_LOGI(TAG, "button 1");
                
                led_state = !led_state;
                gpio_set_level(GPIO_NUM_4, led_state);
                
            }
        }
        
        last_button_state4 = current_button_state;
        
        // 每10ms扫描一次
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
