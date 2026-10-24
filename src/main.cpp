#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c/i2c.h"
#include "oled.h"
#include "bmp280_i2c.h"
#include "spi/spi_slave_app.h"

static const char *TAG = "app";

// Піни та порт шини I2C
#define I2C_MASTER_SCL_IO GPIO_NUM_9
#define I2C_MASTER_SDA_IO GPIO_NUM_8
#define I2C_MASTER_NUM I2C_NUM_0

static bool initialize_bmp280(i2c_master_bus_handle_t bus)
{
    if (bmp280_i2c_hal_init(bus) != BMP280_OK)
    {
        ESP_LOGW(TAG, "BMP280/BME280 I2C device initialization failed");
        return false;
    }

    uint8_t chip_id = 0U;
    if (bmp280_i2c_read_part_number(&chip_id) != BMP280_OK ||
        (chip_id != 0x58U && chip_id != 0x60U))
    {
        ESP_LOGW(TAG, "BMP280/BME280 not found or invalid chip ID: 0x%02X", chip_id);
        return false;
    }

    if (bmp280_i2c_reset() != BMP280_OK)
    {
        ESP_LOGW(TAG, "BMP280 reset failed");
        return false;
    }
    bmp280_i2c_hal_ms_delay(10U);

    if (bmp280_i2c_set_calib() != BMP280_OK ||
        bmp280_i2c_write_config_filter(FILTER_4) != BMP280_OK)
    {
        ESP_LOGW(TAG, "BMP280 calibration/configuration failed");
        return false;
    }

    bmp280_ctrl_meas_t measurement = {};
    measurement.osrs_tmp = OSRS_x1;
    measurement.osrs_press = OSRS_x1;
    measurement.pmode = POWERMODE_NORMAL;

    if (bmp280_i2c_write_osrs(measurement) != BMP280_OK ||
        bmp280_i2c_write_power_mode(POWERMODE_NORMAL) != BMP280_OK)
    {
        ESP_LOGW(TAG, "BMP280 measurement setup failed");
        return false;
    }

    ESP_LOGI(TAG, "%s ready",
             chip_id == 0x60U ? "BME280 (temperature/pressure only)" : "BMP280");
    return true;
}

extern "C" void app_main()
{
    spi_slave_app_start();

    i2c_master_bus_handle_t bus = NULL;
    i2c_bus_settings_t bus_cfg = {};
    bus_cfg.port = I2C_MASTER_NUM;
    bus_cfg.sda_pin = I2C_MASTER_SDA_IO;
    bus_cfg.scl_pin = I2C_MASTER_SCL_IO;
    bus_cfg.enable_internal_pullup = true;
    ESP_ERROR_CHECK(i2c_bus_init(&bus_cfg, &bus));
    const bool bmp280_ready = initialize_bmp280(bus);
    /*
        oled_settings_t oled_cfg = OLED_SETTINGS_128X64_DEFAULT();
        oled_handle_t oled = NULL;
        esp_err_t err = oled_init(bus, &oled_cfg, &oled);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "OLED init failed: %s", esp_err_to_name(err));
            return;
        }

        // Малюємо просту фігуру на OLED 128x64.
        ESP_ERROR_CHECK(oled_clear(oled));
        ESP_ERROR_CHECK(oled_print(oled, 0, 1, false, "OLED drawing"));
        ESP_ERROR_CHECK(oled_draw_line(oled, 64, 14, 40, 34, false));
        ESP_ERROR_CHECK(oled_draw_line(oled, 64, 14, 88, 34, false));
        ESP_ERROR_CHECK(oled_draw_rectangle(oled, 45, 33, 38, 27, false));
        ESP_ERROR_CHECK(oled_fill_rectangle(oled, 59, 44, 10, 15, false));
        ESP_ERROR_CHECK(oled_draw_circle(oled, 64, 27, 4, false));
    */
    while (1)
    {

        bmp280_data_t measurement = {};
        if (bmp280_i2c_read_data(&measurement) == BMP280_OK)
        {

            if (spi_slave_app_update(measurement.temperature) != ESP_OK)
            {
                ESP_LOGW(TAG, "SPI temperature transfer failed");
            }

            ESP_LOGI(TAG, "BMP280/BME280: temperature=%ld centi-C, pressure=%lu Q24.8 Pa",
                     (long)measurement.temperature,
                     (unsigned long)measurement.pressure);
        }
        else
        {
            ESP_LOGW(TAG, "BMP280/BME280 measurement read failed");
        }

        // printf("Temperature: %ld C\n", (long)measurement.temperature);

        // vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
