#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"  // 用于微秒级延时
#include "init.h"
#include "gpio.h"
#include "esp_task_wdt.h"
#include "soc/gpio_num.h"
/*
#define IN1_PIN GPIO_NUM_21
#define IN2_PIN GPIO_NUM_19
#define IN3_PIN GPIO_NUM_18
#define IN4_PIN GPIO_NUM_5*/
#define STEP_DELAY_US 2000
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
    printf("step_forward_start\n");
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
    printf("step_forward_end\n");
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
    printf("step_backward_start\n");
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
    
    printf("step_backward_end\n");
    vTaskDelete(NULL);
}