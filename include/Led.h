#include <stdio.h>
#include "driver/gpio.h"

class Led
{
public:
    void Init(gpio_config_t* gpio_led_conf);
    const gpio_num_t LED_OUT = GPIO_NUM_16;
};
