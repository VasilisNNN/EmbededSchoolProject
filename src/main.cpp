#include <Arduino.h>
#include <atomic>
#include <Config.h>
#include <Led.h>
#include <main.h>

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
uint32_t timer;

void setup()
{
	Serial.begin(115200);

	GreenLed.Init(Config::PIN_GREEN);
	YellowLed.Init(Config::PIN_YELLOW);
	RedLed.Init(Config::PIN_RED);

	timer = millis();
}

void loop()
{
	PhaseChange();
	PhaseManager();
}

void PhaseChange()
{
	uint32_t currentTime = millis();
	

	if (timer + Config::CANPASS_DURATION > currentTime)
	{
		trafficState = TrafficLightState::CanPass;
	}
	else if (timer + Config::FINISHPASSING_DURATION  > currentTime)
	{

		trafficState = TrafficLightState::FinishPassing;
	}
	else if (timer + Config::CANNOTPASS_DURATION > currentTime)
	{

		trafficState = TrafficLightState::CanNotPass;
	}
	else if (timer + Config::FULLSTOP_DURATION > currentTime)
	{

		trafficState = TrafficLightState::FullStop;
	}
		else if (timer + Config::GETREADY_DURATION > currentTime)
	{

		trafficState = TrafficLightState::GetReady;
	}
	else if (timer + 17000 < currentTime)timer = millis();

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
