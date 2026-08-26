#pragma once

#include <stdint.h>

class Config
{
public:
    static constexpr uint8_t PIN_OUT = 2;

    static constexpr uint8_t BLINK_COUNT = 3;
    static constexpr uint32_t BLINK_TIME_MS = 100;
    static constexpr uint32_t DEBOUNCE_TIME_MS = 50;
};
