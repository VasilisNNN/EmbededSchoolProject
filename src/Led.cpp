#include "Led.h"
#include <Arduino.h>

u_int8_t pinID;

void Init(u_int8_t pinId)
{
    pinID = pinId;
    pinMode(pinId, OUTPUT);
   
}

void Set(LedState state)
{
    if (state == LedState::On)
    {
        digitalWrite(pinID, HIGH);
    }
    else
    {
        digitalWrite(pinID, LOW);
    }
}