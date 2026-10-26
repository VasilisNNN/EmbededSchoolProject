#include <cstdio>
#include <cstring>

#include "main.h"
#include "i2c/i2c.h"
#include "lib/at24c32/at24c32.h"
#include "printf/usb_printf.h"

extern "C" void main_cpp(void)
{
    constexpr uint32_t i2c_clock_hz = 100000U;
    constexpr uint32_t eeprom_timeout_ms = 100U;
    constexpr uint16_t test_address = 0U;
    constexpr char expected[] = "HELLO";

    uint8_t write_page[at24c32::page_size];
    uint8_t read_page[at24c32::page_size];
    std::memset(write_page, 0xFF, sizeof(write_page));
    std::memcpy(write_page, expected, sizeof(expected));

    HAL_Delay(2000U);

    I2C_HandleTypeDef i2c1 = {};
    HAL_StatusTypeDef status = I2C_Init(&i2c1, I2C1, i2c_clock_hz);
    if (status != HAL_OK)
    {
        printf("I2C init failed: %d\r\n", static_cast<int>(status));
        return;
    }

    at24c32::At24C32 eeprom(&i2c1, at24c32::default_i2c_address);
    status = eeprom.probe(eeprom_timeout_ms);
    if (status != HAL_OK)
    {
        printf("EEPROM probe failed: %d\r\n", static_cast<int>(status));
        return;
    }

    status = eeprom.write_page(test_address, write_page, eeprom_timeout_ms);
    if (status != HAL_OK)
    {
        printf("EEPROM write failed");
        return;
    }

    status = eeprom.read_page(test_address, read_page, eeprom_timeout_ms);
    if (status != HAL_OK)
    {
        printf("EEPROM read failed");
        return;
    }

    if (std::memcmp(read_page, expected, sizeof(expected)) != 0)
    {
        printf("EEPROM verify failed\r\n");
        return;
    }

    printf("EEPROM write/read OK: %s\r\n", expected);
    while (1)
    {
        HAL_Delay(1000U);
    }
}
