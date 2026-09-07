#include "Arduino.h"
#include "Led.h"
#include "Config.h"

Led led;
float timer;
void setup()
{
    Serial.begin(115200);

    led.Init(Config::PIN_OUT);
    timer = millis();
}


void loop()
{
    const uint32_t now = millis();
    if (timer > now) {
        if (timer - Config::BLINK_TIME_MS < now) {
            led.Set(LedState::Off);
        } else {
            led.Set(LedState::On);
        }
     } else {
         timer =  now + Config::BLINK_TIME_MS * 2;
     }


  
    
}


