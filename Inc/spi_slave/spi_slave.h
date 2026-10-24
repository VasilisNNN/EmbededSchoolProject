#ifndef SPI_SLAVE_H
#define SPI_SLAVE_H

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef SPI_Slave_Init(SPI_HandleTypeDef *hspi,
                                 SPI_TypeDef *instance);
HAL_StatusTypeDef SPI_Slave_DeInit(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef SPI_Slave_Transmit(SPI_HandleTypeDef *hspi,
                                     uint8_t *data,
                                     uint16_t size,
                                     uint32_t timeout);
HAL_StatusTypeDef SPI_Slave_Receive(SPI_HandleTypeDef *hspi,
                                    uint8_t *data,
                                    uint16_t size,
                                    uint32_t timeout);
HAL_StatusTypeDef SPI_Slave_TransmitReceive(SPI_HandleTypeDef *hspi,
                                            uint8_t *tx_data,
                                            uint8_t *rx_data,
                                            uint16_t size,
                                            uint32_t timeout);
HAL_StatusTypeDef SPI_Slave_StartTemperatureStream(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef SPI_Slave_SetTemperature(int16_t temperature_tenths_c);
HAL_StatusTypeDef SPI_Slave_GetTemperatureStreamStatus(void);
void SPI_Slave_IRQHandler(void);
void SPI_Slave_OnTransmitReceiveComplete(SPI_HandleTypeDef *hspi);
void SPI_Slave_OnError(SPI_HandleTypeDef *hspi);

#ifdef __cplusplus
}
#endif

#endif /* SPI_SLAVE_H */
