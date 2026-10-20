#include <stdio.h>

#include "main.h"
#include "i2c/i2c.h"
#include "oled/oled.h"
#include "ds1307/ds1307.h"
#include "bme280/bme280.h"

#define I2C_SPEED_HZ 100000U
#define DISPLAY_UPDATE_INTERVAL_MS 1000U

static I2C_HandleTypeDef hi2c1;

static void PrintDisplayLine(OLED_HandleTypeDef *oled, uint8_t row, const char *text)
{
    OLED_Print(oled, row, 1, false, text);
}

extern "C" void main_cpp()
{
    HAL_StatusTypeDef status = I2C_Init(&hi2c1, I2C1, I2C_SPEED_HZ);
    if (status != HAL_OK)
    {
        printf("[I2C] Initialization failed (status=%d)\r\n", (int)status);
        return;
    }

    OLED_SettingsTypeDef oled_settings = OLED_SETTINGS_128X64_DEFAULT();
    OLED_HandleTypeDef *oled = NULL;

    status = OLED_Init(&hi2c1, &oled_settings, &oled);
    if (status != HAL_OK)
    {
        printf("[OLED] Initialization failed (status=%d)\r\n", (int)status);
        return;
    }

    DS1307_HandleTypeDef *rtc = NULL;
    DS1307_Init(&hi2c1, &rtc);

    BME280_HandleTypeDef *bme280 = NULL;
    BME280_Init(&hi2c1, &bme280);

    uint32_t last_update_tick = HAL_GetTick();

    while (1)
    {
        const uint32_t now = HAL_GetTick();
        if ((uint32_t)(now - last_update_tick) < DISPLAY_UPDATE_INTERVAL_MS)
        {
            continue;
        }
        last_update_tick = now;

        char date_text[16] = "DATE: RTC ERROR";
        char time_text[12] = "RTC ERROR";
        char temperature_text[16] = "T: BME ERROR";
        char humidity_text[16] = "RH: BME ERROR";
        char pressure_text[24] = "P: BME ERROR";

        DS1307_DateTimeTypeDef datetime;

        DS1307_GetDateTime(rtc, &datetime);

        snprintf(date_text, sizeof(date_text), "%02u/%02u/%04u",
                 datetime.day,
                 datetime.month,
                 datetime.year);
        snprintf(time_text, sizeof(time_text), "%02u:%02u:%02u",
                 datetime.hour,
                 datetime.minute,
                 datetime.second);

        BME280_MeasurementTypeDef measurement;
        BME280_Read(bme280, &measurement);

        const int temperature = measurement.temperature_tenths_c;
        const int temperature_abs =
            (int)((temperature < 0) ? -temperature : temperature);
            
        snprintf(temperature_text, sizeof(temperature_text), "T: %s%d.%d C",
                 (temperature < 0) ? "-" : "",
                 (int)(temperature_abs / 10U),
                 (int)(temperature_abs % 10U));
        snprintf(humidity_text, sizeof(humidity_text), "RH: %u.%u %%",
                 measurement.humidity_tenths_percent / 10U,
                 measurement.humidity_tenths_percent % 10U);
        snprintf(pressure_text, sizeof(pressure_text), "P: %u.%u hPa",
                 measurement.pressure_tenths_hpa / 10U,
                 measurement.pressure_tenths_hpa % 10U);

        OLED_Clear(oled);
        

        PrintDisplayLine(oled, 0, "CLOCK / BME280");
        PrintDisplayLine(oled, 1, date_text);
        OLED_Print(oled, 2, 2, false, time_text);
       
        PrintDisplayLine(oled, 5, temperature_text);
        PrintDisplayLine(oled, 6, humidity_text);
        PrintDisplayLine(oled, 7, pressure_text);

        printf("[DISPLAY] DATE=%s TIME=%s | %s | %s | %s\r\n",
               date_text, time_text, temperature_text, humidity_text, pressure_text);
    }
}
