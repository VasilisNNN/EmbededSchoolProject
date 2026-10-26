#ifndef AT24C32_H
#define AT24C32_H

#include "stm32f4xx_hal.h"

namespace at24c32
{
constexpr uint16_t device_size = 4096U;
constexpr uint16_t page_size = 32U;
constexpr uint16_t page_count = device_size / page_size;
constexpr uint16_t default_i2c_address = 0x50U;

class At24C32
{
public:
    At24C32(I2C_HandleTypeDef *i2c, uint16_t i2c_address);

    HAL_StatusTypeDef probe(uint32_t timeout_ms) const;
    HAL_StatusTypeDef read_page(uint16_t address,
                                uint8_t *data,
                                uint32_t timeout_ms) const;
    HAL_StatusTypeDef write_page(uint16_t address,
                                 const uint8_t *data,
                                 uint32_t timeout_ms) const;

private:
    I2C_HandleTypeDef *i2c_;
    uint16_t i2c_address_;
};
}

#endif
