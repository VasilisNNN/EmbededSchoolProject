#include "printf/usb_printf.h"
#include "main.h"

/* Retarget the low-level write syscall to USB CDC so printf() goes over Type-C. */
int _write(int file, char *ptr, int len)
{
  (void)file;

  if (len == 0)
  {
    return 0;
  }

  if ((ptr == NULL) || (len < 0))
  {
    return -1;
  }

  int sent = 0;

  while (sent < len)
  {
    const uint16_t chunk = (len - sent > APP_TX_DATA_SIZE)
                               ? APP_TX_DATA_SIZE
                               : (uint16_t)(len - sent);
    const uint32_t start_tick = HAL_GetTick();
    uint8_t result = CDC_Transmit_FS((uint8_t *)&ptr[sent], chunk);
    while (result == USBD_BUSY)
    {
      if ((HAL_GetTick() - start_tick) > USB_PRINTF_TX_TIMEOUT_MS)
      {
        return (sent > 0) ? sent : -1;
      }

      HAL_Delay(1);
      result = CDC_Transmit_FS((uint8_t *)&ptr[sent], chunk);
    }

    if (result != USBD_OK)
    {
      return (sent > 0) ? sent : -1;
    }

    sent += chunk;
  }

  return sent;
}

/* Fallback character output used by some libc configurations for printf(). */
int __io_putchar(int ch)
{
    char c = (char)ch;
    _write(1, &c, 1);

    return ch;
}
