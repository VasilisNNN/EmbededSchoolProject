#include <stdio.h>
#include "driver/gpio.h"

class Button
{
public:
     void Init(gpio_config_t* gpio_button_conf);
     const gpio_num_t BUTTON_IN = GPIO_NUM_15;

};
