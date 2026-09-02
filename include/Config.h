#pragma once

#include <stdint.h>

class Config
{
    public:
    static constexpr uint8_t PIN_OUT_0 = 5;
    static constexpr uint8_t PIN_OUT_1 = 6;
    static constexpr uint8_t PIN_OUT_2 = 7;


    static constexpr uint32_t BLINK_TIME_MS_0 = 100;
    static constexpr uint32_t BLINK_TIME_MS_1 = 500;
    static constexpr uint32_t BLINK_TIME_MS_2 = 1000;
};
