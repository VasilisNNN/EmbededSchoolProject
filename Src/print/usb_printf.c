#include "printf/usb_printf.h"
#include "uart/uart.h"

int _write(int file, char *ptr, int len)
{
    (void)file;

    uint8_t buffer[64];
    uint16_t buffered = 0;

    for (int i = 0; i < len; i++)
    {
      if ((ptr[i] == '\n') && ((i == 0) || (ptr[i - 1] != '\r')))
      {
        buffer[buffered++] = '\r';
        if (buffered == sizeof(buffer))
        {
          if (UART_ConsoleWrite_DMA(buffer, buffered, UART_PRINTF_TX_TIMEOUT_MS) != HAL_OK)
          {
            return -1;
          }
          buffered = 0;
        }
      }

      buffer[buffered++] = (uint8_t)ptr[i];
      if (buffered == sizeof(buffer))
      {
        if (UART_ConsoleWrite_DMA(buffer, buffered, UART_PRINTF_TX_TIMEOUT_MS) != HAL_OK)
        {
          return -1;
        }
        buffered = 0;
      }
    }

    if ((buffered > 0) &&
        (UART_ConsoleWrite_DMA(buffer, buffered, UART_PRINTF_TX_TIMEOUT_MS) != HAL_OK))
    {
      return -1;
    }

    return len;
}

int __io_putchar(int ch)
{
    char c = (char)ch;
    if (_write(1, &c, 1) < 0)
    {
      return -1;
    }

    return ch;
}
