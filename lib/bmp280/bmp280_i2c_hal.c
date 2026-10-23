#include "bmp280_i2c_hal.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c/i2c.h"

#define BMP280_I2C_ADDRESS 0x76U
#define BMP280_I2C_SPEED_HZ 100000U
#define BMP280_I2C_TIMEOUT_MS 1000U

static i2c_master_dev_handle_t bmp280_device;

int16_t bmp280_i2c_hal_init(i2c_master_bus_handle_t bus)
{
    if (bus == NULL || bmp280_device != NULL) {
        return BMP280_ERR;
    }

    const i2c_device_settings_t settings = {
        .address = BMP280_I2C_ADDRESS,
        .scl_speed_hz = BMP280_I2C_SPEED_HZ,
    };

    return i2c_device_add(bus, &settings, &bmp280_device) == ESP_OK
               ? BMP280_OK
               : BMP280_ERR;
}

int16_t bmp280_i2c_hal_read(uint8_t address,
                            uint8_t *reg,
                            uint8_t *data,
                            uint16_t count)
{
    if (bmp280_device == NULL || address != BMP280_I2C_ADDRESS ||
        reg == NULL || data == NULL || count == 0U) {
        return BMP280_ERR;
    }

    return i2c_read_register(bmp280_device, *reg, data, count,
                             BMP280_I2C_TIMEOUT_MS) == ESP_OK
               ? BMP280_OK
               : BMP280_ERR;
}

int16_t bmp280_i2c_hal_write(uint8_t address, uint8_t *data, uint16_t count)
{
    if (bmp280_device == NULL || address != BMP280_I2C_ADDRESS ||
        data == NULL || count < 2U) {
        return BMP280_ERR;
    }

    return i2c_write_register(bmp280_device, data[0], &data[1], count - 1U,
                              BMP280_I2C_TIMEOUT_MS) == ESP_OK
               ? BMP280_OK
               : BMP280_ERR;
}

void bmp280_i2c_hal_ms_delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}
