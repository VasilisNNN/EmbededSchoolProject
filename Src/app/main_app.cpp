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
    const HAL_StatusTypeDef status = OLED_Print(oled, row, 1, false, text);
    if (status != HAL_OK)
    {
        printf("[OLED] Row %u update failed (status=%d)\r\n", (unsigned)row, (int)status);
    }
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
    const HAL_StatusTypeDef rtc_init_status = DS1307_Init(&hi2c1, &rtc);
    if (rtc_init_status != HAL_OK)
    {
        printf("[RTC] DS1307 initialization failed (status=%d)\r\n", (int)rtc_init_status);
    }

    BME280_HandleTypeDef *bme280 = NULL;
    const HAL_StatusTypeDef bme_init_status = BME280_Init(&hi2c1, &bme280);
    if (bme_init_status != HAL_OK)
    {
        printf("[BME280] Initialization failed (status=%d); check wiring and address\r\n",
               (int)bme_init_status);
    }
    else
    {
        printf("[BME280] Initialized\r\n");
    }

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
        if (rtc_init_status == HAL_OK)
        {
            status = DS1307_GetDateTime(rtc, &datetime);
            if (status == HAL_OK)
            {
                snprintf(date_text, sizeof(date_text), "%02u/%02u/%04u",
                         (unsigned)datetime.day,
                         (unsigned)datetime.month,
                         (unsigned)datetime.year);
                snprintf(time_text, sizeof(time_text), "%02u:%02u:%02u",
                         (unsigned)datetime.hour,
                         (unsigned)datetime.minute,
                         (unsigned)datetime.second);
            }
            else
            {
                printf("[RTC] Read failed (status=%d)\r\n", (int)status);
            }
        }

        BME280_MeasurementTypeDef measurement;
        if (bme_init_status == HAL_OK)
        {
            status = BME280_Read(bme280, &measurement);
            if (status == HAL_OK)
            {
                const int temperature = measurement.temperature_tenths_c;
                const unsigned int temperature_abs =
                    (unsigned int)((temperature < 0) ? -temperature : temperature);
                snprintf(temperature_text, sizeof(temperature_text), "T: %s%d.%d C",
                         (temperature < 0) ? "-" : "",
                         (int)(temperature_abs / 10U),
                         (int)(temperature_abs % 10U));
                snprintf(humidity_text, sizeof(humidity_text), "RH: %u.%u %%",
                         (unsigned)(measurement.humidity_tenths_percent / 10U),
                         (unsigned)(measurement.humidity_tenths_percent % 10U));
                snprintf(pressure_text, sizeof(pressure_text), "P: %u.%u hPa",
                         (unsigned)(measurement.pressure_tenths_hpa / 10U),
                         (unsigned)(measurement.pressure_tenths_hpa % 10U));
            }
            else
            {
                printf("[BME280] Read failed (status=%d)\r\n", (int)status);
            }
        }

        status = OLED_Clear(oled);
        if (status != HAL_OK)
        {
            printf("[OLED] Clear failed (status=%d)\r\n", (int)status);
        }

        PrintDisplayLine(oled, 0, "CLOCK / BME280");
        PrintDisplayLine(oled, 1, date_text);
        status = OLED_Print(oled, 2, 2, false, time_text);
        if (status != HAL_OK)
        {
            printf("[OLED] Time update failed (status=%d)\r\n", (int)status);
        }
        PrintDisplayLine(oled, 5, temperature_text);
        PrintDisplayLine(oled, 6, humidity_text);
        PrintDisplayLine(oled, 7, pressure_text);

        printf("[DISPLAY] DATE=%s TIME=%s | %s | %s | %s\r\n",
               date_text, time_text, temperature_text, humidity_text, pressure_text);
    }
}
