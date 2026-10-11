#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "main_app.h"
#include "printf/usb_printf.h"
#include "main_app.h"

extern UART_HandleTypeDef huart1;
#define BUTTON_IN = 6;

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
char send_buffer[2];
uint32_t ind = 0;

extern "C" void ALT_MAIN_Init()
{
}

extern "C" void main_cpp()
{
    send_buffer[0] = 0;
    send_buffer[1] = 1;

    while (1)
    {

        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == 0)
        {
            printf("send_buffer %c\n", send_buffer);
            UART_SendText(send_buffer);
        }
        else
        {
            UART_SendText("0");

        }

        if (HAL_UART_Receive(
                &huart1,
                &received_byte,
                1,
                HAL_MAX_DELAY) == HAL_OK)
        {

            printf("received_byte %c\n", (char)received_byte);

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
