#include "printf/uart_printf.h"
#include "uart/uart.h"

#include <limits.h>
#include <stdint.h>

int _write(int file, char *ptr, int len)
{
  (void)file;

  if ((ptr == NULL) || (len < 0))
  {
    return -1;
  }

  int sent = 0;
  while (sent < len)
  {
    int remaining = len - sent;
    uint16_t chunk = (remaining > UINT16_MAX) ? UINT16_MAX : (uint16_t)remaining;
    HAL_StatusTypeDef status =
        UART_Monitor_Write_DMA((uint8_t *)&ptr[sent], chunk,
                               UART_PRINTF_TX_TIMEOUT_MS);
    if (status != HAL_OK)
    {
      return (sent > 0) ? sent : -1;
    }

    sent += chunk;
  }

  return sent;
}

int __io_putchar(int ch)
{
  char c = (char)ch;
  _write(1, &c, 1);

  return ch;
}
