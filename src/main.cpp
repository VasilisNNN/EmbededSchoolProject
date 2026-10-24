#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "spi.h"
#include "led.h"

#define LED_OUT GPIO_NUM_16

static const char *TAG = "BME280_SPI";

extern spi_device_handle_t spi;

extern "C" void app_main()
{
    Led led(LED_OUT);

    esp_err_t ret;

    ret = spi_initialize(&spi);
    ESP_ERROR_CHECK(ret);

    while (1)
    {
        int16_t temperature_tenths_c = 0;
        spi_read_temperature_tenths_c(spi, &temperature_tenths_c);

        const int32_t magnitude =
            temperature_tenths_c < 0
                ? -(int32_t)temperature_tenths_c
                : (int32_t)temperature_tenths_c;


        ESP_LOGI(TAG, "STM32 temperature: %s%ld.%ld C",
                 temperature_tenths_c < 0 ? "-" : "",
                 (long)(magnitude / 10), (long)(magnitude % 10));

       
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
