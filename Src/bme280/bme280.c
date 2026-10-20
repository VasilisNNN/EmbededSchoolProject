#include "bme280/bme280.h"

#include <stdlib.h>

#define BME280_ADDRESS_PRIMARY 0x76U
#define BME280_ADDRESS_SECONDARY 0x77U
#define BME280_REGISTER_ID 0xD0U
#define BME280_REGISTER_RESET 0xE0U
#define BME280_REGISTER_CTRL_HUM 0xF2U
#define BME280_REGISTER_STATUS 0xF3U
#define BME280_REGISTER_CTRL_MEAS 0xF4U
#define BME280_REGISTER_CONFIG 0xF5U
#define BME280_REGISTER_PRESSURE 0xF7U
#define BME280_REGISTER_CALIBRATION_TP 0x88U
#define BME280_REGISTER_CALIBRATION_H1 0xA1U
#define BME280_REGISTER_CALIBRATION_H2 0xE1U
#define BME280_TIMEOUT_MS 100U
#define BME280_CHIP_ID 0x60U

struct BME280_HandleTypeDef
{
  I2C_HandleTypeDef *i2c;
  uint16_t address;
  uint16_t dig_t1;
  int16_t dig_t2;
  int16_t dig_t3;
  uint16_t dig_p1;
  int16_t dig_p2;
  int16_t dig_p3;
  int16_t dig_p4;
  int16_t dig_p5;
  int16_t dig_p6;
  int16_t dig_p7;
  int16_t dig_p8;
  int16_t dig_p9;
  uint8_t dig_h1;
  int16_t dig_h2;
  uint8_t dig_h3;
  int16_t dig_h4;
  int16_t dig_h5;
  int8_t dig_h6;
  int32_t t_fine;
};

static uint16_t BME280_ReadU16LE(const uint8_t *data)
{
  return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

static int16_t BME280_ReadS16LE(const uint8_t *data)
{
  return (int16_t)BME280_ReadU16LE(data);
}

static int16_t BME280_SignExtend12(uint16_t value)
{
  if ((value & 0x0800U) != 0U)
  {
    value |= 0xF000U;
  }
  return (int16_t)value;
}

static HAL_StatusTypeDef BME280_WriteRegister(BME280_HandleTypeDef *sensor,
                                              uint8_t reg,
                                              uint8_t value)
{
  uint8_t packet[2] = {reg, value};
  return I2C_Transmit(sensor->i2c, sensor->address, packet, sizeof(packet),
                      BME280_TIMEOUT_MS);
}

static HAL_StatusTypeDef BME280_ReadCalibration(BME280_HandleTypeDef *sensor)
{
  uint8_t calib_tp[26];
  uint8_t calib_h[7];
  uint8_t calib_h1;

  HAL_StatusTypeDef status =
      I2C_ReadRegister(sensor->i2c, sensor->address, BME280_REGISTER_CALIBRATION_TP,
                       calib_tp, sizeof(calib_tp), BME280_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    return status;
  }

  status = I2C_ReadRegister(sensor->i2c, sensor->address, BME280_REGISTER_CALIBRATION_H1,
                            &calib_h1, 1U, BME280_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    return status;
  }

  status = I2C_ReadRegister(sensor->i2c, sensor->address, BME280_REGISTER_CALIBRATION_H2,
                            calib_h, sizeof(calib_h), BME280_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    return status;
  }

  sensor->dig_t1 = BME280_ReadU16LE(&calib_tp[0]);
  sensor->dig_t2 = BME280_ReadS16LE(&calib_tp[2]);
  sensor->dig_t3 = BME280_ReadS16LE(&calib_tp[4]);
  sensor->dig_p1 = BME280_ReadU16LE(&calib_tp[6]);
  sensor->dig_p2 = BME280_ReadS16LE(&calib_tp[8]);
  sensor->dig_p3 = BME280_ReadS16LE(&calib_tp[10]);
  sensor->dig_p4 = BME280_ReadS16LE(&calib_tp[12]);
  sensor->dig_p5 = BME280_ReadS16LE(&calib_tp[14]);
  sensor->dig_p6 = BME280_ReadS16LE(&calib_tp[16]);
  sensor->dig_p7 = BME280_ReadS16LE(&calib_tp[18]);
  sensor->dig_p8 = BME280_ReadS16LE(&calib_tp[20]);
  sensor->dig_p9 = BME280_ReadS16LE(&calib_tp[22]);
  sensor->dig_h1 = calib_h1;
  sensor->dig_h2 = BME280_ReadS16LE(&calib_h[0]);
  sensor->dig_h3 = calib_h[2];
  sensor->dig_h4 = BME280_SignExtend12((uint16_t)(((uint16_t)calib_h[3] << 4) |
                                                  (calib_h[4] & 0x0FU)));
  sensor->dig_h5 = BME280_SignExtend12((uint16_t)(((uint16_t)calib_h[5] << 4) |
                                                  (calib_h[4] >> 4)));
  sensor->dig_h6 = (int8_t)calib_h[6];

  return HAL_OK;
}

static HAL_StatusTypeDef BME280_Configure(BME280_HandleTypeDef *sensor)
{
  HAL_StatusTypeDef status = BME280_WriteRegister(sensor, BME280_REGISTER_CTRL_HUM, 0x01U);
  if (status != HAL_OK)
  {
    return status;
  }

  status = BME280_WriteRegister(sensor, BME280_REGISTER_CONFIG, 0x00U);
  if (status != HAL_OK)
  {
    return status;
  }

  return BME280_WriteRegister(sensor, BME280_REGISTER_CTRL_MEAS, 0x27U);
}

HAL_StatusTypeDef BME280_Init(I2C_HandleTypeDef *hi2c, BME280_HandleTypeDef **sensor)
{
  uint8_t chip_id;
  uint16_t address;

  if ((hi2c == NULL) || (sensor == NULL))
  {
    return HAL_ERROR;
  }

  *sensor = NULL;
  if (I2C_Probe(hi2c, BME280_ADDRESS_PRIMARY, BME280_TIMEOUT_MS) == HAL_OK)
  {
    address = BME280_ADDRESS_PRIMARY;
  }
  else if (I2C_Probe(hi2c, BME280_ADDRESS_SECONDARY, BME280_TIMEOUT_MS) == HAL_OK)
  {
    address = BME280_ADDRESS_SECONDARY;
  }
  else
  {
    return HAL_ERROR;
  }

  BME280_HandleTypeDef *dev = calloc(1U, sizeof(*dev));
  if (dev == NULL)
  {
    return HAL_ERROR;
  }

  dev->i2c = hi2c;
  dev->address = address;

  HAL_StatusTypeDef status =
      I2C_ReadRegister(hi2c, address, BME280_REGISTER_ID, &chip_id, 1U,
                       BME280_TIMEOUT_MS);
  if ((status == HAL_OK) && (chip_id != BME280_CHIP_ID))
  {
    status = HAL_ERROR;
  }

  if (status == HAL_OK)
  {
    status = BME280_ReadCalibration(dev);
  }

  if ((status == HAL_OK) && (dev->dig_p1 == 0U))
  {
    status = HAL_ERROR;
  }

  if (status == HAL_OK)
  {
    status = BME280_Configure(dev);
  }

  if (status != HAL_OK)
  {
    free(dev);
    return status;
  }

  *sensor = dev;
  return HAL_OK;
}

HAL_StatusTypeDef BME280_Deinit(BME280_HandleTypeDef *sensor)
{
  if (sensor == NULL)
  {
    return HAL_ERROR;
  }

  free(sensor);
  return HAL_OK;
}

HAL_StatusTypeDef BME280_Read(BME280_HandleTypeDef *sensor,
                              BME280_MeasurementTypeDef *measurement)
{
  uint8_t data[8];

  if ((sensor == NULL) || (measurement == NULL))
  {
    return HAL_ERROR;
  }

  HAL_StatusTypeDef status =
      I2C_ReadRegister(sensor->i2c, sensor->address, BME280_REGISTER_PRESSURE,
                       data, sizeof(data), BME280_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    return status;
  }

  const int32_t adc_p = (int32_t)(((uint32_t)data[0] << 12) |
                                  ((uint32_t)data[1] << 4) |
                                  ((uint32_t)data[2] >> 4));
  const int32_t adc_t = (int32_t)(((uint32_t)data[3] << 12) |
                                  ((uint32_t)data[4] << 4) |
                                  ((uint32_t)data[5] >> 4));
  const int32_t adc_h = (int32_t)(((uint32_t)data[6] << 8) | data[7]);

  int32_t var1 = ((((adc_t >> 3) - ((int32_t)sensor->dig_t1 << 1))) *
                  (int32_t)sensor->dig_t2) >> 11;
  int32_t var2 = (((((adc_t >> 4) - (int32_t)sensor->dig_t1) *
                    ((adc_t >> 4) - (int32_t)sensor->dig_t1)) >> 12) *
                  (int32_t)sensor->dig_t3) >> 14;
  sensor->t_fine = var1 + var2;

  const int32_t temperature_centi_c = (sensor->t_fine * 5 + 128) >> 8;
  measurement->temperature_tenths_c =
      (int16_t)((temperature_centi_c >= 0)
                    ? ((temperature_centi_c + 5) / 10)
                    : ((temperature_centi_c - 5) / 10));

  int64_t pressure_var1 = (int64_t)sensor->t_fine - 128000;
  int64_t pressure_var2 = pressure_var1 * pressure_var1 * sensor->dig_p6;
  pressure_var2 += (pressure_var1 * sensor->dig_p5) << 17;
  pressure_var2 += (int64_t)sensor->dig_p4 << 35;
  pressure_var1 = ((pressure_var1 * pressure_var1 * sensor->dig_p3) >> 8) +
                  ((pressure_var1 * sensor->dig_p2) << 12);
  pressure_var1 = ((((int64_t)1 << 47) + pressure_var1) * sensor->dig_p1) >> 33;
  if (pressure_var1 == 0)
  {
    return HAL_ERROR;
  }

  int64_t pressure = 1048576 - adc_p;
  pressure = (((pressure << 31) - pressure_var2) * 3125) / pressure_var1;
  pressure_var1 = ((int64_t)sensor->dig_p9 * (pressure >> 13) *
                   (pressure >> 13)) >> 25;
  pressure_var2 = ((int64_t)sensor->dig_p8 * pressure) >> 19;
  pressure = ((pressure + pressure_var1 + pressure_var2) >> 8) +
             ((int64_t)sensor->dig_p7 << 4);
  measurement->pressure_tenths_hpa =
      (uint32_t)((pressure * 10 + 128000) / 25600);

  int32_t humidity = sensor->t_fine - 76800;
  humidity = (((((adc_h << 14) - ((int32_t)sensor->dig_h4 << 20) -
                 ((int32_t)sensor->dig_h5 * humidity)) +
                16384) >>
               15) *
              (((((((humidity * (int32_t)sensor->dig_h6) >> 10) *
                    (((humidity * (int32_t)sensor->dig_h3) >> 11) + 32768)) >>
                   10) +
                  2097152) *
                   (int32_t)sensor->dig_h2 +
               8192) >>
               14));
  humidity -= (((((humidity >> 15) * (humidity >> 15)) >> 7) *
                (int32_t)sensor->dig_h1) >>
               4);
  if (humidity < 0)
  {
    humidity = 0;
  }
  else if (humidity > 419430400)
  {
    humidity = 419430400;
  }

  measurement->humidity_tenths_percent =
      (uint16_t)((((uint32_t)humidity >> 12) * 100U + 512U) / 1024U);

  return HAL_OK;
}
