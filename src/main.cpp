#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "Led.h"
#include "ADC.h"
#include <iterator>


constexpr int THRESHOLD_LOW  = 30;
constexpr int THRESHOLD_HIGH = 100;
bool ledState = false;

Led led;
ADC adc;

/*static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer,
                                        const gptimer_alarm_event_data_t *edata,
                                        void *user_data)
{
    static bool led_state;
    led_state = !led_state;
    gpio_set_level(led.LED_OUT, led_state);

    return true;
}*/

extern "C" void app_main()
{

    

    adc.Init();

    // Налаштування структури GPIO для LED
    gpio_config_t gpio_led_conf = {};
    led.Init(&gpio_led_conf);

   
   /* gpio_config_t gpio_button_conf = {};
    button.Init(&gpio_button_conf);*/

    

    // Конфігурація GPIO
    gpio_config(&gpio_led_conf);
    //gpio_config(&gpio_button_conf);

    // Встановлення початкового стану
    gpio_set_level(led.LED_OUT, 0);

    while (1)
    {

        printf("ADC SMA value = %d\n", adc.SMA());

        if (adc.SMA() > THRESHOLD_HIGH)
        ledState = false;
        else if (adc.SMA() < THRESHOLD_LOW)
        ledState = true;

        gpio_set_level(led.LED_OUT, ledState);
     
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
