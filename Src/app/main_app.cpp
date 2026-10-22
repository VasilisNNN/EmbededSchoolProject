#include <stdio.h>

#include "main.h"
#include "i2c/i2c.h"
#include "oled/oled.h"
#include "ds1307/ds1307.h"
#include "bme280/bme280.h"
#include "main_app.h"


#define I2C_SPEED_HZ 100000U
#define DISPLAY_UPDATE_INTERVAL_MS 1000U

static I2C_HandleTypeDef hi2c1;

static void PrintDisplayLine(OLED_HandleTypeDef *oled, uint8_t row, const char *text)
{
    OLED_Print(oled, row, 1, false, text);
}


STRING FomatString(int variable, STRING end)
{

    STRING result = "0";
   
    if (variable < 0)
    {
        result += "-";
    }

    result += std::to_string(variable / 10);
    result += ".";
    result += std::to_string(variable % 10);
    result += end;
    return result;
}

extern "C" void main_cpp()
{

    if (I2C_Init(&hi2c1, I2C1, I2C_SPEED_HZ) != HAL_OK)
    {
        printf("I2C error\n");
        return;
    }

    OLED_SettingsTypeDef settings = OLED_SETTINGS_128X64_DEFAULT();
    OLED_HandleTypeDef *oled = NULL;

    if (OLED_Init(&hi2c1, &settings, &oled) != HAL_OK)
    {
        printf("OLED error\n");
        return;
    }

    DS1307_HandleTypeDef *rtc = NULL;
    DS1307_Init(&hi2c1, &rtc);

    BME280_HandleTypeDef *bme280 = NULL;
    BME280_Init(&hi2c1, &bme280);

    while (1)
    {
        DS1307_DateTimeTypeDef date;
        DS1307_GetDateTime(rtc, &date);

        BME280_MeasurementTypeDef data;
        BME280_Read(bme280, &data);

        STRING day = std::to_string(date.day);
        STRING month = std::to_string(date.month);
        STRING year = std::to_string(date.year);

        STRING hour = std::to_string(date.hour);
        STRING minute = std::to_string(date.minute);
        STRING second = std::to_string(date.second);

        if (date.day < 10)
            day = "0" + day;
        if (date.month < 10)
            month = "0" + month;
        if (date.hour < 10)
            hour = "0" + hour;
        if (date.minute < 10)
            minute = "0" + minute;
        if (date.second < 10)
            second = "0" + second;

        STRING dateText = day + "/" + month + "/" + year;
        STRING timeText = hour + ":" + minute + ":" + second;

        OLED_Clear(oled);

        PrintDisplayLine(oled, 0, "CLOCK / BME280");
        PrintDisplayLine(oled, 1, dateText.c_str());
        OLED_Print(oled, 2, 2, false, timeText.c_str());

        PrintDisplayLine(oled, 5, FomatString(data.temperature_tenths_c, " C").c_str());
        PrintDisplayLine(oled, 6, FomatString(data.humidity_tenths_percent, " %").c_str());
        PrintDisplayLine(oled, 7, FomatString(data.pressure_tenths_hpa, " hPa").c_str());

        printf("Date: %s Time: %s\n",
               dateText.c_str(), timeText.c_str());

        HAL_Delay(1000);
    }
}
