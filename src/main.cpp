#include "Arduino.h"
#include "Led.h"
#include "Config.h"

Led led;
float timer;
void setup()
{
    Serial.begin(115200);

    led.Init(Config::PIN_OUT);
  
}


void loop()
{
 
    if(timer > millis() && timer- Config::BLINK_TIME_MS < millis() )
	{
	    led.Set(LedState::Off);
		return;
	}


    led.Set(LedState::On);
         
	if(timer < millis())
    timer =  millis() + Config::BLINK_TIME_MS*2;
    
}


