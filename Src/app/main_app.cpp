#include <cstdio>

#include "main.h"
#include "i2c/i2c.h"
#include "at24c32/at24c32.h"
#include "logger/logger.h"
#include "ssd1306/ssd1306.h"
#include "oled/oled.h"

extern "C" void main_cpp(void)
{
    static I2C_HandleTypeDef hi2c1;
    OLED_SettingsTypeDef settings = OLED_SETTINGS_128X64_DEFAULT();
    OLED_HandleTypeDef *oled = NULL;

    if (OLED_Init(&hi2c1, &settings, &oled) != HAL_OK)
    {
        printf("OLED error\n");
        return;
    }

    constexpr uint32_t i2c_clock_hz = 100000U;
    constexpr char message[] = "HELLO";

    AT24C32_HandleTypeDef eeprom = {};
    Logger logObj(eeprom);

    HAL_Delay(4000U);

    I2C_HandleTypeDef i2c1 = {};
    HAL_StatusTypeDef status = I2C_Init(&i2c1, I2C1, i2c_clock_hz);
    if (status != HAL_OK)
    {
        printf("I2C init failed\r\n");
        return;
    }

    status = AT24C32_Init(&eeprom, &i2c1, 0U);
    if (status != HAL_OK)
    {
        printf("EEPROM init failed\r\n");
        return;
    }

    logObj.Init();

    uint16_t written_number = 0U;
    logObj.Write(message, &written_number);

    char read_message[log_text_capacity];
    status = logObj.Read(written_number,
                         read_message,
                         sizeof(read_message));
    if (status != HAL_OK)
    {
        printf("Logger Read failed\r\n");
        return;
    }

    printf("Logger write/read OK: %s\r\n", read_message);
    OLED_Print(oled, 0, 1, 1, read_message);

    while (1)
    {
        HAL_Delay(1000U);
    }
}
