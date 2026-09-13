#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"

void Init()
{
    esp_err_t err = ESP_OK;

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
  //  timer_callbacks.on_alarm = timer_on_alarm_cb;

    err = gptimer_register_event_callbacks(timer, &timer_callbacks, NULL);
    if (err != ESP_OK)
    {
        printf("Failed to register event callbacks, err = %d\n", err);
        return;
    }

    gptimer_enable(timer);

    gptimer_start(timer);

}