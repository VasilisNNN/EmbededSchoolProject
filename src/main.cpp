#include <Arduino.h>
#include <atomic>
#include <Config.h>
#include <Led.h>
#include <main.h>

Led GreenLed;
Led YellowLed;
Led RedLed;



TrafficLightState trafficState;
uint32_t timer;


void setup()
{
	Serial.begin(115200);

	GreenLed.Init(Config::PIN_GREEN);
	YellowLed.Init(Config::PIN_YELLOW);
	RedLed.Init(Config::PIN_RED);

	
    SwitchPhase(TrafficLightState::CanPass, Config::CANPASS_DURATION);
}

void loop()
{
	
	PhaseManager();
}


 
void SwitchPhase(TrafficLightState state, uint32_t _timercorrection)
{


        timer = millis() + _timercorrection;
		trafficState = state;
		
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

		if(timer < millis())
		{
           SwitchPhase(TrafficLightState::FinishPassing, Config::FINISHPASSING_DURATION);

		}
		break;
	}
	case FinishPassing:
	{
		GreenLed.Blink();
		if(timer < millis())
		 SwitchPhase(TrafficLightState::CanNotPass, Config::CANPASS_DURATION);

		
		break;
	}
	case CanNotPass:
	{
		GreenLed.LedOff();
		YellowLed.LedOn();
        
		if(timer < millis())
		 SwitchPhase(TrafficLightState::FullStop, Config::FULLSTOP_DURATION);

		break;
	}
	case FullStop:
	{
		GreenLed.LedOff();
		YellowLed.LedOff();
		RedLed.LedOn();

		if(timer < millis())
		 SwitchPhase(TrafficLightState::GetReady, Config::GETREADY_DURATION);

		break;
	}
	case GetReady:
	{
		GreenLed.LedOff();
		YellowLed.LedOn();
		RedLed.LedOn();

		if(timer < millis())
		 SwitchPhase(TrafficLightState::CanPass, Config::CANPASS_DURATION);

		break;
	}
	default:
		break;
	}
}
