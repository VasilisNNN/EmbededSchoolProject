#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "main_app.h"
#include "printf/usb_printf.h"
#include "main_app.h"

extern UART_HandleTypeDef huart1;

void UART_SendText(const char *text)
{
    HAL_UART_Transmit(
        &huart1,
        (uint8_t *)text,
        strlen(text),
        HAL_MAX_DELAY);
}

uint8_t received_byte;
char buffer[100];
uint32_t ind = 0;

extern "C" void ALT_MAIN_Init()
{
    
}

extern "C" void main_cpp()
{

    while (1)
    {

        if (HAL_UART_Receive(
                &huart1,
                &received_byte,
                1,
                HAL_MAX_DELAY) == HAL_OK)
        {
            if (received_byte == '\r' || received_byte == '\n')
            {
                buffer[ind] = '\0';

                UART_SendText("Received: ");
                UART_SendText(buffer);
                UART_SendText("\r\n");

                ind = 0;
            }
            else if (ind < sizeof(buffer) - 1)
            {
                buffer[ind++] = (char)received_byte;
            }
        }
    }

}
