#include <stdio.h>
#include "driver/gpio.h"

class Led
{
public:
     gpio_num_t LED_PIN = GPIO_NUM_10;
    void Init(gpio_config_t *gpio_led_conf, gpio_num_t led_pin);
};
