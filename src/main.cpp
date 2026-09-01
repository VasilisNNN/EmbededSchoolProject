#include "Arduino.h"
#include "Led.h"
#include "Config.h"

LedState Light_State;
Led led;

void setup()
{
    Serial.begin(115200);

    led.Init(Config::PIN_OUT);
  

}


void loop()
{
    static u_int32_t timer;

    if(timer > millis() && timer- Config::BLINK_TIME_MS < millis() )
	{
	    led.Set(LedState::Off);
		return;
	}


    led.Set(LedState::On);
         
	if(timer < millis())
    timer =  millis() + Config::BLINK_TIME_MS*2;
    
}


