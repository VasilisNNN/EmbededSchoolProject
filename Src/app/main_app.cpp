#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "uart/uart.h"

static UART_HandleTypeDef huart1;
static uint32_t adc_dma_value;


extern "C" void main_cpp()
{
    HAL_StatusTypeDef status = UART_Init(&huart1, USART1, 115200);
    if (status != HAL_OK)
    {
        printf("[UART] USART1 initialization failed (status=%d)\n", (int)status);
        Error_Handler();
    }

    HAL_ADC_Start_DMA(&hadc1, &adc_dma_value, 1);

    while (1)
    {

        const uint32_t millivolts = (adc_dma_value * 3300U) / 4095U;
        printf("ADC: %lu\r\n",
               (unsigned long)adc_dma_value);

        HAL_Delay(100U);
        HAL_ADC_Start_DMA(&hadc1, &adc_dma_value, 1);
    }
}
