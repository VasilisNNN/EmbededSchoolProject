#include "Button.h"
#include <stdio.h>
#include "driver/gpio.h"


void Button::Init(gpio_config_t* gpio_button_conf)
{
    gpio_button_conf->pin_bit_mask = 1ULL << BUTTON_IN;
    gpio_button_conf->mode = GPIO_MODE_INPUT;
    gpio_button_conf->pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_button_conf->pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_button_conf->intr_type = GPIO_INTR_DISABLE;

}