#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "Led.h"
#include "Button.h"
#include "ADC.h"
#include <iterator>

const int SMA_threshold = 70;

Led led;
Button button;
ADC adc;

static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer,
                                        const gptimer_alarm_event_data_t *edata,
                                        void *user_data)
{
    static bool led_state;
    led_state = !led_state;
    gpio_set_level(led.LED_OUT, led_state);

    return true;
}

extern "C" void app_main()
{

    esp_err_t err = ESP_OK;

    adc.Init();

    // Налаштування структури GPIO для LED
    gpio_config_t gpio_led_conf = {};
    led.Init(&gpio_led_conf);

    // Налаштування структури GPIO для BUTTON
    gpio_config_t gpio_button_conf = {};
    button.Init(&gpio_button_conf);

    // Об'єкт таймера
    gptimer_handle_t timer;
    // Конфігурація таймера
    gptimer_config_t timer_config = {};
    timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000; // 1 MHz
    // Створення нового таймера
    err = gptimer_new_timer(&timer_config, &timer);
    if (err != ESP_OK)
    {
        printf("Failed to create timer, err = %d\n", err);
        return;
    }

    // Конфігурація аларму таймера
    gptimer_alarm_config_t alarm_config = {};
    alarm_config.alarm_count = 1000000; // Тривалість 1 с
    alarm_config.reload_count = 0;
    alarm_config.flags.auto_reload_on_alarm = true;

    // Створення аларму таймера
    err = gptimer_set_alarm_action(timer, &alarm_config);
    if (err != ESP_OK)
    {
        printf("Failed to set alarm action, err = %d\n", err);
        return;
    }

    gptimer_event_callbacks_t timer_callbacks = {};
    timer_callbacks.on_alarm = timer_on_alarm_cb;

    err = gptimer_register_event_callbacks(timer, &timer_callbacks, NULL);
    if (err != ESP_OK)
    {
        printf("Failed to register event callbacks, err = %d\n", err);
        return;
    }

    gptimer_enable(timer);

    gptimer_start(timer);

    // Конфігурація GPIO
    gpio_config(&gpio_led_conf);
    gpio_config(&gpio_button_conf);

    // Встановлення початкового стану
    gpio_set_level(led.LED_OUT, 0);

    while (1)
    {

        printf("ADC SMA value = %d\n", adc.SMA());

     
        if (adc.SMA() > SMA_threshold)
            gpio_set_level(led.LED_OUT, true);
        else
            gpio_set_level(led.LED_OUT, false);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
