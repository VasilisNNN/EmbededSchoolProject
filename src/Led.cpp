#include <Led.h>
#include <Arduino.h>
#include <Config.h>



void Led::Init(uint8_t id)
{
    LedID = id;
    pinMode(id, OUTPUT);
    digitalWrite(LedID, LOW);
}
void Led::LedOn()
{

    digitalWrite(LedID, HIGH);
}

void Led::LedOff()
{
    digitalWrite(LedID, LOW);
}

void Led::Blink()
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
