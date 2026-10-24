#include <stdio.h>

#include "main.h"
#include "i2c/i2c.h"
#include "oled/oled.h"
#include "ds1307/ds1307.h"
#include "bme280/bme280.h"
#include "spi_slave/spi_slave.h"
#include "main_app.h"


#define I2C_SPEED_HZ 100000U
#define DISPLAY_UPDATE_INTERVAL_MS 1000U

static I2C_HandleTypeDef hi2c1;
static SPI_HandleTypeDef hspi2;

static void PrintDisplayLine(OLED_HandleTypeDef *oled, uint8_t row, const char *text)
{
    OLED_Print(oled, row, 1, false, text);
}


STRING FomatString(int variable, STRING end)
{

    STRING result = " ";
   
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

    BME280_MeasurementTypeDef initial_measurement;
    HAL_StatusTypeDef initial_measurement_status =
        BME280_Read(bme280, &initial_measurement);
    if (initial_measurement_status == HAL_OK)
    {
        SPI_Slave_SetTemperature(initial_measurement.temperature_tenths_c);
    }

    if (SPI_Slave_Init(&hspi2, SPI2) != HAL_OK)
    {
        printf("[SPI] SPI2 slave initialization failed\n");
        return;
    }
    if (SPI_Slave_StartTemperatureStream(&hspi2) != HAL_OK)
    {
        printf("[SPI] Temperature stream start failed\n");
        return;
    }
    if (initial_measurement_status != HAL_OK)
    {
        printf("[BME280] Initial measurement failed (status=%d)\n",
               (int)initial_measurement_status);
    }

    DS1307_DateTimeTypeDef initialTime = {2026, 10, 20, 3, 20, 56, 0};
    DS1307_SetDateTime(rtc, &initialTime);


    while (1)
    {
        DS1307_DateTimeTypeDef date;
        DS1307_GetDateTime(rtc, &date);

        BME280_MeasurementTypeDef data;
        HAL_StatusTypeDef measurement_status = BME280_Read(bme280, &data);

        if (measurement_status == HAL_OK)
        {
            if (SPI_Slave_SetTemperature(data.temperature_tenths_c) != HAL_OK)
            {
                printf("[SPI] Temperature update failed\n");
            }
        }
        else
        {
            printf("[BME280] Measurement failed (status=%d)\n",
                   (int)measurement_status);
        }

        HAL_StatusTypeDef spi_status =
            SPI_Slave_GetTemperatureStreamStatus();
        if (spi_status != HAL_OK)
        {
            printf("[SPI] Temperature stream failed (status=%d)\n",
                   (int)spi_status);
        }

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

        PrintDisplayLine(oled, 0, "Clock / Weather");
        PrintDisplayLine(oled, 1, dateText.c_str());
        OLED_Print(oled, 2, 2, false, timeText.c_str());

        if (measurement_status == HAL_OK)
        {
            PrintDisplayLine(oled, 5, FomatString(data.temperature_tenths_c, " C").c_str());
            PrintDisplayLine(oled, 6, FomatString(data.humidity_tenths_percent, " %").c_str());
            PrintDisplayLine(oled, 7, FomatString(data.pressure_tenths_hpa, " hPa").c_str());
        }
        else
        {
            PrintDisplayLine(oled, 5, "BME280 read error");
        }

        printf("Date: %s Time: %s\n",
               dateText.c_str(), timeText.c_str());

        HAL_Delay(DISPLAY_UPDATE_INTERVAL_MS);
    }
}