#include <Arduino.h>

class Config
{
public:
static constexpr uint8_t FAN_PIN = 17;

static constexpr uint32_t PERIOD_MS = 1000000;
static constexpr uint32_t FAN_ON_TIME_MS = 300000;

static constexpr uint32_t TIMER_PRESCALER = 80;
};
