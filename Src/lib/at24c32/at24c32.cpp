#include "lib/at24c32/at24c32.h"

#include <cstring>

namespace at24c32
{
namespace
{
constexpr uint16_t last_valid_address = device_size - page_size;
}

At24C32::At24C32(I2C_HandleTypeDef *i2c, uint16_t i2c_address)
    : i2c_(i2c), i2c_address_(i2c_address)
{
}

HAL_StatusTypeDef At24C32::probe(uint32_t timeout_ms) const
{
    if ((i2c_ == nullptr) || (i2c_address_ < 0x50U) ||
        (i2c_address_ > 0x57U))
    {
        return HAL_ERROR;
    }

    return HAL_I2C_IsDeviceReady(i2c_,
                                 static_cast<uint16_t>(i2c_address_ << 1U),
                                 1U,
                                 timeout_ms);
}

HAL_StatusTypeDef At24C32::read_page(uint16_t address,
                                    uint8_t *data,
                                    uint32_t timeout_ms) const
{
    if ((i2c_ == nullptr) || (data == nullptr) ||
        (i2c_address_ < 0x50U) || (i2c_address_ > 0x57U) ||
        ((address % page_size) != 0U) || (address > last_valid_address))
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Mem_Read(i2c_,
                            static_cast<uint16_t>(i2c_address_ << 1U),
                            address,
                            I2C_MEMADD_SIZE_16BIT,
                            data,
                            page_size,
                            timeout_ms);
}

HAL_StatusTypeDef At24C32::write_page(uint16_t address,
                                     const uint8_t *data,
                                     uint32_t timeout_ms) const
{
    if ((i2c_ == nullptr) || (data == nullptr) ||
        ((address % page_size) != 0U) || (address > last_valid_address) ||
        (i2c_address_ < 0x50U) || (i2c_address_ > 0x57U))
    {
        return HAL_ERROR;
    }

    uint8_t page[page_size];
    std::memcpy(page, data, sizeof(page));

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
        i2c_,
        static_cast<uint16_t>(i2c_address_ << 1U),
        address,
        I2C_MEMADD_SIZE_16BIT,
        page,
        sizeof(page),
        timeout_ms);
    if (status != HAL_OK)
    {
        return status;
    }

    return HAL_I2C_IsDeviceReady(i2c_,
                                 static_cast<uint16_t>(i2c_address_ << 1U),
                                 10U,
                                 timeout_ms);
}
}
