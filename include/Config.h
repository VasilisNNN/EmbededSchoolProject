#include <Arduino.h>


class Config
{
  public:
   static constexpr u_int8_t PIN_BUTTON = 12;
   static constexpr uint32_t DEBOUNCE_TIME = 50;
   static constexpr uint32_t POLL_INTERVAL = 10;
};
