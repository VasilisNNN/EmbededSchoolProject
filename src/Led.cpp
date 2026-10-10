#include "Led.h"
#include <stdio.h>
#include "driver/gpio.h"



void Led::Init(gpio_config_t* gpio_led_conf, gpio_num_t led_pin)
{
    gpio_led_conf->pin_bit_mask = 1ULL << led_pin;
    gpio_led_conf->mode = GPIO_MODE_OUTPUT;
    gpio_led_conf->pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_led_conf->pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_led_conf->intr_type = GPIO_INTR_DISABLE;

}
