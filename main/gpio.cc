#include "gpio.h"
void led_init(gpio_num_t num);
void button_init(gpio_num_t num);
void IRAM_ATTR button_isr_handler(void* arg);
void led_task(void *pvParameters);
void screen_button_task(void *pvParameters);
void button_task1(void *pvParameters);
void button_task2(void *pvParameters);