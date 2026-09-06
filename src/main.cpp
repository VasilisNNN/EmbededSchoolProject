#include <Arduino.h>
#include <atomic>
#include <Config.h>
#include <Led.h>

Led GreenLed;
Led YellowLed;
Led RedLed;

enum TrafficLightState
{
	CanPass,
	FinishPassing,
	CanNotPass,
	FullStop,
	GetReady,
};

TrafficLightState trafficState;

void setup()
{
	Serial.begin(115200);

	GreenLed.Init(Config::PIN_GREEN);
	YellowLed.Init(Config::PIN_YELLOW);
	RedLed.Init(Config::PIN_RED);
}

void loop()
{
    PhaseChange();
    PhaseManager();
}


void PhaseChange()
{

}


void PhaseManager()
{
	switch (trafficState)
	{
	case CanPass:
	{
        GreenLed.LedOn();
		YellowLed.LedOff();
		RedLed.LedOff();
		break;
	}
	case FinishPassing:
	{
         GreenLed.Blink();
		break;
	}
	case CanNotPass:
	{
        GreenLed.LedOff();
		YellowLed.LedOn();
		
		break;
	}
	case FullStop:
	{
        GreenLed.LedOff();
		YellowLed.LedOff();
		RedLed.LedOn();
		break;
	}
	case GetReady:
	{ 
		GreenLed.LedOff();
		YellowLed.LedOn();
		RedLed.LedOn();

		break;
	}
	default:
		break;
	}
}
