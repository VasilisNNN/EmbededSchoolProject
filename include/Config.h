#include <Arduino.h>

class Config
{
public:
static constexpr uint8_t PIN_GREEN = 15;
static constexpr uint8_t PIN_YELLOW = 16;
static constexpr uint8_t PIN_RED = 17;

static constexpr uint32_t BLINK_DELAY = 500;

static constexpr uint32_t CANPASS_DURATION = 5000;
static constexpr uint32_t FINISHPASSING_DURATION = 3000;
static constexpr uint32_t CANNOTPASS_DURATION = 2000;
static constexpr uint32_t FULLSTOP_DURATION = 5000;
static constexpr uint32_t GETREADY_DURATION = 2000;


};
