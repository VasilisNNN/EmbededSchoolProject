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
static SPI_HandleTypeDef hspi1;

#define ESP_SPI_CS_GPIO_Port GPIOA
#define ESP_SPI_CS_Pin GPIO_PIN_4

static HAL_StatusTypeDef SPI_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();

    HAL_GPIO_WritePin(ESP_SPI_CS_GPIO_Port, ESP_SPI_CS_Pin, GPIO_PIN_SET);

    GPIO_InitTypeDef gpio = {};
    gpio.Pin = ESP_SPI_CS_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(ESP_SPI_CS_GPIO_Port, &gpio);

    gpio.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &gpio);

    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;

    return HAL_SPI_Init(&hspi1);
}

static HAL_StatusTypeDef ReadESP_SPI(uint8_t *received, uint16_t size)
{
    uint8_t request[4] = {0U, 0U, 0U, 0U};

    if (received == NULL || size != sizeof(request))
    {
        return HAL_ERROR;
    }

    HAL_GPIO_WritePin(ESP_SPI_CS_GPIO_Port, ESP_SPI_CS_Pin, GPIO_PIN_RESET);
    const HAL_StatusTypeDef status =
        HAL_SPI_TransmitReceive(&hspi1, request, received, size, 100U);
    HAL_GPIO_WritePin(ESP_SPI_CS_GPIO_Port, ESP_SPI_CS_Pin, GPIO_PIN_SET);

    return status;
}

static int32_t DecodeTemperatureMilliC(const uint8_t data[4])
{
    const uint32_t raw = ((uint32_t)data[0] << 24) |
                         ((uint32_t)data[1] << 16) |
                         ((uint32_t)data[2] << 8) |
                         (uint32_t)data[3];

    if ((raw & 0x80000000U) != 0U)
    {
        return (int32_t)((int64_t)raw - 0x100000000LL);
    }

    return (int32_t)raw;
}

static STRING FormatTemperatureMilliC(int32_t temperature_milli_c)
{
    const int64_t absolute_value = temperature_milli_c < 0
                                       ? -(int64_t)temperature_milli_c
                                       : temperature_milli_c;
    const int64_t tenths_c = (absolute_value + 50) / 100;
    char text[24];
    snprintf(text, sizeof(text), "ESP: %s%ld.%ld C",
             temperature_milli_c < 0 ? "-" : "",
             (long)(tenths_c / 10),
             (long)(tenths_c % 10));
    return STRING(text);
}

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

    const bool spi_ready = SPI_Init() == HAL_OK;
    if (!spi_ready)
    {
        printf("SPI initialization failed: error=0x%08lX\n",
               (unsigned long)hspi1.ErrorCode);
    }

    DS1307_HandleTypeDef *rtc = NULL;
    DS1307_Init(&hi2c1, &rtc);

    BME280_HandleTypeDef *bme280 = NULL;
    BME280_Init(&hi2c1, &bme280);

    DS1307_DateTimeTypeDef initialTime = {2026, 10, 20, 3, 20, 56, 0};
    DS1307_SetDateTime(rtc, &initialTime);

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

        PrintDisplayLine(oled, 0, "Clock / Weather");
        PrintDisplayLine(oled, 1, dateText.c_str());
        OLED_Print(oled, 2, 2, false, timeText.c_str());

        uint8_t spi_data[4] = {};
        if (!spi_ready)
        {
            PrintDisplayLine(oled, 5, "ESP: SPI error");
        }
        else
        {
            const HAL_StatusTypeDef spi_status = ReadESP_SPI(spi_data, sizeof(spi_data));
            if (spi_status != HAL_OK)
            {
                PrintDisplayLine(oled, 5, "ESP: SPI error");
                printf("ESP SPI read failed: status=%d error=0x%08lX\n",
                       spi_status, (unsigned long)hspi1.ErrorCode);
            }
            else
            {
                const int32_t temperature_milli_c = DecodeTemperatureMilliC(spi_data);
                if (temperature_milli_c < -40000 || temperature_milli_c > 85000)
                {
                    char raw_text[17];
                    snprintf(raw_text, sizeof(raw_text), "RX:%02X %02X %02X %02X",
                             spi_data[0], spi_data[1], spi_data[2], spi_data[3]);
                    PrintDisplayLine(oled, 5, raw_text);
                    printf("ESP SPI invalid temp: bytes=%02X %02X %02X %02X value=%ld mC\n",
                           spi_data[0], spi_data[1], spi_data[2], spi_data[3],
                           (long)temperature_milli_c);
                }
                else
                {
                    const STRING temperature_text = FormatTemperatureMilliC(temperature_milli_c);
                    PrintDisplayLine(oled, 5, temperature_text.c_str());
                    printf("ESP temperature: %s (%ld mC)\n",
                           temperature_text.c_str(), (long)temperature_milli_c);
                }
            }

        }

        PrintDisplayLine(oled, 6, FomatString(data.humidity_tenths_percent, " %").c_str());
        PrintDisplayLine(oled, 7, FomatString(data.pressure_tenths_hpa, " hPa").c_str());
        HAL_Delay(DISPLAY_UPDATE_INTERVAL_MS);
    }
}
