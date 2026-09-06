#include <Arduino.h>

class Led
{
private:
    /* data */
public:
uint32_t LedID;
void Init(uint8_t id);
void LedOn();
void LedOff();
void Blink();
};
