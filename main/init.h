#ifndef __INIT_H__
#define __INIT_H__
#include "soc/gpio_num.h"
static const char *TAG = "zjhnb";
typedef struct {
    gpio_num_t gpio_num;
    int time; //延迟
    int ledctrl;//0:关闭 1:打开 2:无
} led_config;
#endif