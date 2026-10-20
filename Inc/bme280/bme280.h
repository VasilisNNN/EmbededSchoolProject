#ifndef BME280_H
#define BME280_H

#include <stdint.h>

#include "i2c/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BME280_HandleTypeDef BME280_HandleTypeDef;

typedef struct
{
  int16_t temperature_tenths_c;
  uint16_t humidity_tenths_percent;
  uint32_t pressure_tenths_hpa;
} BME280_MeasurementTypeDef;

/* Finds a BME280 at I2C address 0x76 or 0x77 and starts normal measurements. */
HAL_StatusTypeDef BME280_Init(I2C_HandleTypeDef *hi2c, BME280_HandleTypeDef **sensor);

HAL_StatusTypeDef BME280_Deinit(BME280_HandleTypeDef *sensor);

HAL_StatusTypeDef BME280_Read(BME280_HandleTypeDef *sensor,
                              BME280_MeasurementTypeDef *measurement);

#ifdef __cplusplus
}
#endif

#endif /* BME280_H */
