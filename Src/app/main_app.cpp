#include "main.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdio.h>
#include <stdint.h>

extern I2C_HandleTypeDef hi2c1;

#define DS1307_ADDR (0x68 << 1)

typedef struct
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t weekday;
} RTC_DateTime;

/* DS1307 weekday register: 1 = Sunday, 7 = Saturday */
static const char *weekdays[] =
{
    "???", "Sun", "Mon", "Tue",
    "Wed", "Thu", "Fri", "Sat"
};

static uint8_t BCD_To_Dec(uint8_t value)
{
    return ((value >> 4) * 10U) + (value & 0x0FU);
}

/* 1. Initialize OLED */
static void Display_Init(void)
{
    SSD1306_Init();
    SSD1306_Fill(Black);
    SSD1306_UpdateScreen();
}

/* 2. Read date and time from DS1307 */
static HAL_StatusTypeDef RTC_Read(RTC_DateTime *dt)
{
    uint8_t data[7];

    if (HAL_I2C_Mem_Read(
            &hi2c1,
            DS1307_ADDR,
            0x00,
            I2C_MEMADD_SIZE_8BIT,
            data,
            7,
            100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Clock halted */
    if (data[0] & 0x80U)
        return HAL_ERROR;

    dt->seconds = BCD_To_Dec(data[0] & 0x7FU);
    dt->minutes = BCD_To_Dec(data[1] & 0x7FU);

    /* Support both 12-hour and 24-hour modes */
    if (data[2] & 0x40U)
    {
        uint8_t h = BCD_To_Dec(data[2] & 0x1FU);
        uint8_t pm = (data[2] & 0x20U) != 0U;

        dt->hours = (h % 12U) + (pm ? 12U : 0U);
    }
    else
    {
        dt->hours = BCD_To_Dec(data[2] & 0x3FU);
    }

    dt->weekday = BCD_To_Dec(data[3] & 0x07U);
    dt->day     = BCD_To_Dec(data[4] & 0x3FU);
    dt->month   = BCD_To_Dec(data[5] & 0x1FU);
    dt->year    = 2000U + BCD_To_Dec(data[6]);

    return HAL_OK;
}

/* 3. Update OLED */
static void Display_Update(const RTC_DateTime *dt)
{
    char timeStr[12];
    char dateStr[24];

    snprintf(timeStr, sizeof(timeStr),
             "%02u:%02u:%02u",
             (unsigned)dt->hours,
             (unsigned)dt->minutes,
             (unsigned)dt->seconds);

    const char *weekday =
        (dt->weekday >= 1U && dt->weekday <= 7U)
        ? weekdays[dt->weekday] : "???";

    snprintf(dateStr, sizeof(dateStr),
             "%s %02u.%02u.%04u",
             weekday,
             (unsigned)dt->day,
             (unsigned)dt->month,
             (unsigned)dt->year);

    SSD1306_Fill(Black);

    SSD1306_GotoXY(10, 10);
    SSD1306_Puts(timeStr, &Font_11x18, White);

    SSD1306_GotoXY(0, 40);
    SSD1306_Puts(dateStr, &Font_7x10, White);

    SSD1306_UpdateScreen();
}

/* Main application */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();

    Display_Init();

    RTC_DateTime now;

    while (1)
    {
        if (RTC_Read(&now) == HAL_OK)
        {
            Display_Update(&now);
        }
        else
        {
            SSD1306_Fill(Black);
            SSD1306_GotoXY(0, 20);
            SSD1306_Puts("RTC ERROR", &Font_7x10, White);
            SSD1306_UpdateScreen();
        }

        HAL_Delay(1000);
    }
}


STM32CubeMX config
