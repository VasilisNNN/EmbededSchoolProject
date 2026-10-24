#include "spi_slave_app.h"

#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_slave.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SPI_SLAVE_HOST SPI2_HOST
#define SPI_SLAVE_PIN_MOSI GPIO_NUM_11
#define SPI_SLAVE_PIN_MISO GPIO_NUM_13
#define SPI_SLAVE_PIN_SCLK GPIO_NUM_12
#define SPI_SLAVE_PIN_CS GPIO_NUM_10
#define SPI_SLAVE_TRANSACTION_SIZE 4U

static const char *TAG = "spi_slave";
static DMA_ATTR uint8_t tx_data[SPI_SLAVE_TRANSACTION_SIZE];

static DMA_ATTR uint8_t rx_data[SPI_SLAVE_TRANSACTION_SIZE];

esp_err_t spi_slave_app_update(int32_t data)
{
    const int64_t temperature_milli_c = (int64_t)data * 10;
    const uint32_t encoded_temperature = (uint32_t)temperature_milli_c;
    tx_data[0] = (uint8_t)(encoded_temperature >> 24);
    tx_data[1] = (uint8_t)(encoded_temperature >> 16);
    tx_data[2] = (uint8_t)(encoded_temperature >> 8);
    tx_data[3] = (uint8_t)encoded_temperature;

    spi_slave_transaction_t transaction = {0};
    transaction.length = SPI_SLAVE_TRANSACTION_SIZE * 8U;
    transaction.tx_buffer = tx_data;
    transaction.rx_buffer = rx_data;

    const esp_err_t err =
        spi_slave_transmit(SPI_SLAVE_HOST, &transaction, portMAX_DELAY);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPI transaction failed: %s", esp_err_to_name(err));
    }

    return err;
}

esp_err_t spi_slave_app_start(void)
{
    const spi_bus_config_t bus_config = {
        .mosi_io_num = SPI_SLAVE_PIN_MOSI,
        .miso_io_num = SPI_SLAVE_PIN_MISO,
        .sclk_io_num = SPI_SLAVE_PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = SPI_SLAVE_TRANSACTION_SIZE,
    };
    const spi_slave_interface_config_t slave_config = {
        .spics_io_num = SPI_SLAVE_PIN_CS,
        .flags = 0,
        .queue_size = 1,
        .mode = 0,
        .post_setup_cb = NULL,
        .post_trans_cb = NULL,
    };

    esp_err_t err =
        spi_slave_initialize(SPI_SLAVE_HOST, &bus_config, &slave_config,
                             SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPI slave init failed: %s", esp_err_to_name(err));
        return err;
    }

    /*if (xTaskCreate(spi_slave_task, "spi_slave", 3072, NULL, 5, NULL) !=
        pdPASS) {
        spi_slave_free(SPI_SLAVE_HOST);
        ESP_LOGE(TAG, "Could not create SPI slave task");
        return ESP_ERR_NO_MEM;
    }*/

    ESP_LOGI(TAG, "SPI2 slave ready (mode 0, 4-byte transactions)");
    return ESP_OK;
}
