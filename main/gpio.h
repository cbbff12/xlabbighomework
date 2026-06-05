#include "hal/gpio_types.h"
#include "soc/gpio_num.h"
extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"    
#include "esp_log.h"    
#include <string.h>                  
}
#include "init.h"
typedef struct {
    gpio_num_t gpio_num;
    int time; //延迟
    int ledctrl;//0:关闭 1:打开 2:无
} led_config;
#define LED_GPIO GPIO_NUM_2
#define DEBOUNCE_DELAY_MS  50// 按钮消抖延时（毫秒）

static int led_state = 0;
static bool last_button_state = true; 

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
void led_task(void *pvParameters)
{
    led_config *led1_config = (led_config *)pvParameters;
    gpio_num_t gpio_num = led1_config->gpio_num;
    int time = led1_config->time;
    int ledctrl = led1_config->ledctrl;
    bool current_button_state;
    uint32_t last_interrupt_time = 0;
    
    while (1) {
        // 读取当前按钮状态（GPIO35上拉，按下为低电平）
        current_button_state = gpio_get_level(gpio_num);
        printf("%d\n", current_button_state);
        // 检测下降沿（从高电平变为低电平，即按钮按下）
        if (current_button_state == 0 && last_button_state == 1) {
            // 简单的软件消抖
            uint32_t current_time = xTaskGetTickCount();
            if ((current_time - last_interrupt_time) > pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)) {
                ESP_LOGI(TAG, "button 4");
                // 有效按下，切换LED状态
                led_state = !led_state;
                gpio_set_level(GPIO_NUM_4, led_state);
                printf("Button pressed! LED state changed to: %d\n", led_state);
                
                last_interrupt_time = current_time;
            }
        }
        
        last_button_state = current_button_state;
        
        // 每10ms扫描一次
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

