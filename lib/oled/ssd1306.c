
#include "ssd1306.h"

#include <string.h>
#include "esp_err.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef SSD1306_I2C_ADDR
#define SSD1306_I2C_ADDR 0x3C
#endif

#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH 128
#endif

#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT 64
#endif

#define SSD1306_CONTROL_CMD  0x00
#define SSD1306_CONTROL_DATA 0x40

static esp_err_t ssd1306_write(
    i2c_port_t port,
    uint8_t control,
    const uint8_t *data,
    size_t length)
{
    if (data == NULL || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    return i2c_master_write_to_device(
        port,
        SSD1306_I2C_ADDR,
        (uint8_t[]){ control },
        1,
        pdMS_TO_TICKS(100)
    ) == ESP_OK
        ? ESP_OK
        : ESP_FAIL;
}

esp_err_t ssd1306_send_command(i2c_port_t port, uint8_t command)
{
    return ssd1306_write(port, SSD1306_CONTROL_CMD, &command, 1);
}

esp_err_t ssd1306_send_data(
    i2c_port_t port,
    const uint8_t *data,
    size_t length)
{
    if (data == NULL || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t buffer[SSD1306_WIDTH + 1];

    while (length > 0) {
        size_t chunk = length > SSD1306_WIDTH
            ? SSD1306_WIDTH
            : length;

        buffer[0] = SSD1306_CONTROL_DATA;
        memcpy(&buffer[1], data, chunk);

        esp_err_t err = i2c_master_write_to_device(
            port,
            SSD1306_I2C_ADDR,
            buffer,
            chunk + 1,
            pdMS_TO_TICKS(100)
        );

        if (err != ESP_OK) {
            return err;
        }

        data += chunk;
        length -= chunk;
    }

    return ESP_OK;
}

esp_err_t ssd1306_init(i2c_port_t port)
{
    const uint8_t init_commands[] = {
        0xAE,                   // Display OFF
        0xD5, 0x80,             // Clock divide
        0xA8, SSD1306_HEIGHT-1, // Multiplex ratio
        0xD3, 0x00,             // Display offset
        0x40,                   // Start line
        0x8D, 0x14,             // Charge pump
        0x20, 0x00,             // Horizontal addressing
        0xA1,                   // Segment remap
        0xC8,                   // COM scan direction
        0xDA, SSD1306_HEIGHT == 64 ? 0x12 : 0x02,
        0x81, 0x7F,             // Contrast
        0xD9, 0xF1,             // Pre-charge
        0xDB, 0x40,             // VCOMH
        0xA4,                   // Display follows RAM
        0xA6,                   // Normal display
        0xAF                    // Display ON
    };

    for (size_t i = 0; i < sizeof(init_commands); i++) {
        esp_err_t err = ssd1306_send_command(port, init_commands[i]);
        if (err != ESP_OK) {
            return err;
        }
    }

    return ESP_OK;
}

esp_err_t ssd1306_clear(i2c_port_t port)
{
    uint8_t zeroes[SSD1306_WIDTH] = {0};

    for (uint8_t page = 0; page < SSD1306_HEIGHT / 8; page++) {
        esp_err_t err;

        err = ssd1306_send_command(port, 0xB0 | page);
        if (err != ESP_OK) {
            return err;
        }

        err = ssd1306_send_command(port, 0x00);
        if (err != ESP_OK) {
            return err;
        }

        err = ssd1306_send_command(port, 0x10);
        if (err != ESP_OK) {
            return err;
        }

        err = ssd1306_send_data(port, zeroes, sizeof(zeroes));
        if (err != ESP_OK) {
            return err;
        }
    }

    return ESP_OK;
}
