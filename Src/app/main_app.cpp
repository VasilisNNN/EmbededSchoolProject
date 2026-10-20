#include <stdio.h>

#include "main.h"
#include "i2c/i2c.h"
#include "oled/oled.h"
#include "ds1307/ds1307.h"

#define I2C_SPEED_HZ 100000U

static I2C_HandleTypeDef hi2c1;

extern "C" void main_cpp()
{
    HAL_StatusTypeDef status = I2C_Init(&hi2c1, I2C1, I2C_SPEED_HZ);
    if (status != HAL_OK)
    {
        printf("initialization failed (status=%d)\r\n", (int)status);
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

    DS1307_DateTimeTypeDef initialTime = {2026, 10, 20, 3, 20, 56, 0};
    DS1307_SetDateTime(rtc, &initialTime);

    while (1)
    {
        OLED_Clear(oled);

        OLED_Print(oled, 0, 1, false, "DS1307 CLOCK");

        DS1307_DateTimeTypeDef datetime;
        DS1307_GetDateTime(rtc, &datetime);


        
        OLED_Printf(oled, 2, 2, "%02u:%02u:%02u",
                    datetime.hour,
                    datetime.minute,
                    datetime.second);

        OLED_Printf(oled, 6, 1, "%02u/%02u/%04u",
                    datetime.day,
                    datetime.month,
                    datetime.year);

        HAL_Delay(1000);
    }
}
