#include <Arduino.h>

class Config
{
public:
static constexpr uint8_t FAN_PIN = 17;

static constexpr uint32_t PERIOD_MS = 10000;
static constexpr uint32_t FAN_ON_TIME_MS = 3000;

};
