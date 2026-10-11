#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "uart.h"
#include "led.h"
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

    led.Init(&gpio_led_conf, GPIO_NUM_10);
    button.Init(&gpio_button_conf);

    const esp_err_t init_err = uart_init();

    if (init_err != ESP_OK)
    {
        ESP_LOGE(TAG, "UART initialization failed: %s", esp_err_to_name(init_err));
        return;
    }

    ESP_LOGI(TAG, "UART echo application started");

    uint8_t received_byte;

    while (1)
    {

        if (gpio_get_level(button.BUTTON_IN) == 0)
        {
            uart_transmit((uint8_t *)info, strlen(info));

            printf("PRESSED");
        }

        const esp_err_t receive_err = uart_receive(&received_byte, 1, 0);
        /*if (receive_err == ESP_OK)
        {

            // print recieved bytes
            ESP_LOGI(TAG, "UART RX: 0x%02X \"%c\"",
                     (unsigned int)received_byte, (int)received_byte);

       
        }
        else if (receive_err != ESP_ERR_TIMEOUT)
        {
            ESP_LOGE(TAG, "UART receive error: %s", esp_err_to_name(receive_err));
        }*/

        //  vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
