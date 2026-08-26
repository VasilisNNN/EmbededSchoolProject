#include "Arduino.h"
#include "Led.h"


LedState Light_State;
Led led;

void setup()
{
    Serial.begin(115200);

    led.Init();
  

}


void loop()
{
    static u_int32_t timer;

    if(timer > millis())return;

    led.Set(LedState::On);
         

    
}


