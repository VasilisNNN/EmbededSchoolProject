#ifndef SPI_SLAVE_APP_H
#define SPI_SLAVE_APP_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t spi_slave_app_start(void);
esp_err_t spi_slave_app_update(int32_t data);

#ifdef __cplusplus
}
#endif

#endif
