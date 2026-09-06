#include <Led.h>
#include <Arduino.h>
#include <Config.h>

uint32_t LedID;

void Init(uint8_t id)
{
    LedID = id;
    pinMode(id, OUTPUT);
    digitalWrite(id, LOW);
}
void LedOn()
{

    digitalWrite(LedID, HIGH);
}

void LedOff()
{
    digitalWrite(LedID, LOW);
}

void Blink()
{
    static uint32_t timer = 0;
    static bool state = false;

    if (timer < millis())
    {
        state = !state;
        timer = millis() + Config::BLINK_DELAY;
    }

    if (state)
        digitalWrite(LedID, HIGH);
    else
        digitalWrite(LedID, LOW);
}
