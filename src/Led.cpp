#include "Led.h"
#include <Arduino.h>


constexpr int PIN_OUT  = 2;

void Init()
{
  
    pinMode(PIN_OUT, OUTPUT);
   
}

void Set(LedState state)
{
    if (state == LedState::On)
    {
        digitalWrite(PIN_OUT, HIGH);
    }
    else
    {
        digitalWrite(PIN_OUT, LOW);
    }
}