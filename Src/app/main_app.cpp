#include <cstdio>

#include "main.h"
#include "i2c/i2c.h"
#include "at24c32/at24c32.h"
#include "logger/logger.h"
#include "oled/oled.h"
#include "bme280/bme280.h"
#include <cstring>


#define I2C_SPEED_HZ 100000U

char *FormatChar(int variable, const char *end)
{
    static char result[16];

    snprintf(result, sizeof(result), "%d.%d%s",
             variable / 10,
             variable % 10,
             end);

    return result;
}

struct ScreenText
{
    char lines[2][17];
};

static void PrintLog(uint16_t, const char *text, void *)
{
    printf("%s\r\n", text);
}

extern "C" void main_cpp(void)
{
    AT24C32_HandleTypeDef eeprom = {};
    Logger logObj(eeprom);

    HAL_Delay(2000U);

    I2C_HandleTypeDef i2c1 = {};
    HAL_StatusTypeDef status = I2C_Init(&i2c1, I2C1, I2C_SPEED_HZ);
    if (status != HAL_OK)
    {
        printf("I2C init failed\r\n");
        return;
    }

    OLED_SettingsTypeDef settings = OLED_SETTINGS_128X64_DEFAULT();
    OLED_HandleTypeDef *oled = NULL;

    status = OLED_Init(&i2c1, &settings, &oled);
    if (status != HAL_OK)
    {
        printf("OLED init failed: %d\r\n", static_cast<int>(status));
        return;
    }

  
    status = AT24C32_Init(&eeprom, &i2c1, 0U);
    if (status != HAL_OK)
    {
        printf("EEPROM init failed: %d\r\n", static_cast<int>(status));
        return;
    }

    if (!logObj.Init())
    {
        printf("Logger init failed\r\n");
        return;
    }
    
    uint16_t oldest_number = 0U;
    uint16_t newest_number = 0U;
    printf("Stored logs: %u\r\n", static_cast<unsigned>(logObj.count()));
    if (logObj.oldest_number(&oldest_number) &&
        logObj.newest_number(&newest_number))
    {
        printf("Stored range: #%03u..#%03u\r\n",
               static_cast<unsigned>(oldest_number),
               static_cast<unsigned>(newest_number));
       
        if (!logObj.readRange(oldest_number, newest_number, PrintLog, nullptr))
        {
            printf("Logger range read failed\r\n");
        }

    
        if (!logObj.readAll(PrintLog, nullptr))
        {
            printf("Logger read-all failed\r\n");
        }

        char latest_log[log_text_capacity];
     
        if (logObj.Read(newest_number, latest_log, sizeof(latest_log)))
        {
            printf("Latest log: %s\r\n", latest_log);
        }
        else
        {
            printf("Logger read failed\r\n");
        }
    }

    uint16_t written_number = 0U;
    char read_message[log_text_capacity];

    BME280_HandleTypeDef *bme280 = NULL;
    status = BME280_Init(&i2c1, &bme280);
    if (status != HAL_OK)
    {
        printf("BME280 init failed: %d\r\n", static_cast<int>(status));
        return;
    }

    while (1)
    {
     ScreenText screentext = {};


        char message[128] = "";
        BME280_MeasurementTypeDef data = {};
        status = BME280_Read(bme280, &data);
        if (status != HAL_OK)
        {
            printf("BME280 read failed: %d\r\n", static_cast<int>(status));
            HAL_Delay(1000U);
            continue;
        }

        strcat(message, FormatChar(data.temperature_tenths_c, " C"));

        strcat(message, " ");

        strcat(message, FormatChar(data.humidity_tenths_percent, " %"));

         strcat(message, " ");

        strcat(message, FormatChar(data.pressure_tenths_hpa, " hPa"));

        
        if (!logObj.Write(message, &written_number))
        {
            printf("Logger write failed\r\n");
            continue;
        }

       
        if (!logObj.Read(written_number, read_message, sizeof(read_message)))
        {
            OLED_Print(oled, 0U, 1U, false, "Logger Read failed\r\n");
            printf("Logger Read failed\r\n");
            continue;
        }
        printf("%s\r\n", read_message);

        strncpy(screentext.lines[0], read_message, 16);

        if (strlen(read_message) > 16)
        {
            strncpy(screentext.lines[1], read_message + 16, 16);
        }

        OLED_Print(oled, 0U, 1U, false, screentext.lines[0]);
        OLED_Print(oled, 1U, 1U, false, screentext.lines[1]);
        HAL_Delay(1000U);
    }
}
