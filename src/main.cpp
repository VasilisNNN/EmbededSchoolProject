#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "uart.h"
#include "Led.h"
#include "Button.h"
#include <cstring>

static const char *TAG = "app";
Led led;
Button button;
gpio_config_t gpio_led_conf;
gpio_config_t gpio_button_conf;
const char *info = "ESP32: Hello from UART!\r\n";

extern "C" void app_main()
{

    gpio_led_conf = gpio_config_t{};
    gpio_button_conf = gpio_config_t{};

    led.Init(&gpio_led_conf, GPIO_NUM_10);
    gpio_config(&gpio_led_conf);

    button.Init(&gpio_button_conf);
    gpio_config(&gpio_button_conf);

    const esp_err_t init_err = uart_init();

    if (init_err != ESP_OK)
        return;
    

    uint8_t received_byte;

    while (1)
    {

        if (gpio_get_level(button.BUTTON_IN) == 0)
        {
            uart_transmit((uint8_t *)info, strlen(info));
        }

        const esp_err_t receive_err = uart_receive(&received_byte, 1, 0);

        if (receive_err == ESP_OK)
        {
            printf("received_byte: %c\n", (char)received_byte);

            gpio_set_level(led.LED_PIN, 1);
        }
        else
        {
            gpio_set_level(led.LED_PIN, 0);

            
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
