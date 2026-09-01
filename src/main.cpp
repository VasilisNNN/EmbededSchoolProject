#include "Arduino.h"
#include "Led.h"
#include "Config.h"

Led led_0;
Led led_1;
Led led_2;

void setup()
{
    Serial.begin(115200);


    led_0.Init(Config::PIN_OUT_0);
    led_1.Init(Config::PIN_OUT_1);
    led_2.Init(Config::PIN_OUT_2);
}


void loop()
{

  blink(led_0, Config::BLINK_TIME_MS_0);
  blink(led_1, Config::BLINK_TIME_MS_1);
  blink(led_1, Config::BLINK_TIME_MS_2);

}

void blink(Led currectLed, float delay)
{
    if(currectLed.timer > millis() && currectLed.timer - delay < millis() )
	{
	    currectLed.Set(LedState::Off);
		return;
	}

    currectLed.Set(LedState::On);
         
	if(currectLed.timer < millis())
    currectLed.timer =  millis() + delay*2;

}


