#include "spi_slave/spi_slave.h"

static SPI_HandleTypeDef *temperature_stream_spi;
static volatile int16_t latest_temperature_tenths_c;
static uint8_t temperature_tx[2];
static uint8_t temperature_rx[2];
static volatile HAL_StatusTypeDef temperature_stream_status = HAL_OK;

static void SPI_Slave_LoadTemperature(void)
{
  uint16_t temperature = (uint16_t)latest_temperature_tenths_c;
  temperature_tx[0] = (uint8_t)(temperature >> 8);
  temperature_tx[1] = (uint8_t)temperature;
}

HAL_StatusTypeDef SPI_Slave_Init(SPI_HandleTypeDef *hspi,
                                 SPI_TypeDef *instance)
{
  if ((hspi == NULL) || (instance != SPI2))
  {
    return HAL_ERROR;
  }

  hspi->Instance = instance;
  hspi->Init.Mode = SPI_MODE_SLAVE;
  hspi->Init.Direction = SPI_DIRECTION_2LINES;
  hspi->Init.DataSize = SPI_DATASIZE_8BIT;
  hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi->Init.NSS = SPI_NSS_HARD_INPUT;
  hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi->Init.TIMode = SPI_TIMODE_DISABLE;
  hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi->Init.CRCPolynomial = 10;

  HAL_StatusTypeDef status = HAL_SPI_Init(hspi);
  if (status == HAL_OK)
  {
    HAL_NVIC_SetPriority(SPI2_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);
  }
  return status;
}

HAL_StatusTypeDef SPI_Slave_DeInit(SPI_HandleTypeDef *hspi)
{
  if ((hspi == NULL) || (hspi->Instance != SPI2))
  {
    return HAL_ERROR;
  }

  return HAL_SPI_DeInit(hspi);
}

HAL_StatusTypeDef SPI_Slave_Transmit(SPI_HandleTypeDef *hspi,
                                      uint8_t *data,
                                      uint16_t size,
                                      uint32_t timeout)
{
  if ((hspi == NULL) || (data == NULL) || (size == 0))
  {
    return HAL_ERROR;
  }

  return HAL_SPI_Transmit(hspi, data, size, timeout);
}

HAL_StatusTypeDef SPI_Slave_Receive(SPI_HandleTypeDef *hspi,
                                     uint8_t *data,
                                     uint16_t size,
                                     uint32_t timeout)
{
  if ((hspi == NULL) || (data == NULL) || (size == 0))
  {
    return HAL_ERROR;
  }

  return HAL_SPI_Receive(hspi, data, size, timeout);
}

HAL_StatusTypeDef SPI_Slave_TransmitReceive(SPI_HandleTypeDef *hspi,
                                             uint8_t *tx_data,
                                             uint8_t *rx_data,
                                             uint16_t size,
                                             uint32_t timeout)
{
  if ((hspi == NULL) || (tx_data == NULL) || (rx_data == NULL) ||
      (size == 0))
  {
    return HAL_ERROR;
  }

  return HAL_SPI_TransmitReceive(hspi, tx_data, rx_data, size, timeout);
}

HAL_StatusTypeDef SPI_Slave_StartTemperatureStream(SPI_HandleTypeDef *hspi)
{
  if ((hspi == NULL) || (hspi->Instance != SPI2) ||
      (hspi->State != HAL_SPI_STATE_READY) ||
      ((temperature_stream_spi != NULL) &&
       (temperature_stream_spi != hspi)))
  {
    return HAL_ERROR;
  }

  temperature_stream_spi = hspi;
  temperature_stream_status = HAL_OK;
  SPI_Slave_LoadTemperature();
  temperature_stream_status =
      HAL_SPI_TransmitReceive_IT(hspi, temperature_tx, temperature_rx,
                                 sizeof(temperature_tx));
  return temperature_stream_status;
}

HAL_StatusTypeDef SPI_Slave_SetTemperature(int16_t temperature_tenths_c)
{
  latest_temperature_tenths_c = temperature_tenths_c;
  return HAL_OK;
}

HAL_StatusTypeDef SPI_Slave_GetTemperatureStreamStatus(void)
{
  return temperature_stream_status;
}

void SPI_Slave_IRQHandler(void)
{
  if (temperature_stream_spi != NULL)
  {
    HAL_SPI_IRQHandler(temperature_stream_spi);
  }
}

void SPI_Slave_OnTransmitReceiveComplete(SPI_HandleTypeDef *hspi)
{
  if ((temperature_stream_spi == NULL) ||
      (temperature_stream_spi != hspi))
  {
    return;
  }

  SPI_Slave_LoadTemperature();
  temperature_stream_status =
      HAL_SPI_TransmitReceive_IT(hspi, temperature_tx, temperature_rx,
                                 sizeof(temperature_tx));
}

void SPI_Slave_OnError(SPI_HandleTypeDef *hspi)
{
  if ((temperature_stream_spi != NULL) &&
      (temperature_stream_spi == hspi))
  {
    temperature_stream_status = HAL_ERROR;
  }
}
