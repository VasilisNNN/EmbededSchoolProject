#include <Arduino.h>
#include <Config.h>

hw_timer_t *fanTimer = NULL;

volatile bool fanRunning = false;

void IRAM_ATTR onTimer()
{
	fanRunning = !fanRunning;

	if (fanRunning)
	{
		digitalWrite(Config::FAN_PIN, HIGH);
		timerAlarmWrite(fanTimer, Config::FAN_ON_TIME_MS, false);
	}
	else
	{
		digitalWrite(Config::FAN_PIN, LOW);
		timerAlarmWrite(fanTimer, Config::PERIOD_MS - Config::FAN_ON_TIME_MS, false);
	}

	timerAlarmEnable(fanTimer);
}

void setup()
{
	pinMode(Config::FAN_PIN, OUTPUT);
	digitalWrite(Config::FAN_PIN, LOW);

	fanTimer = timerBegin(0, Config::TIMER_PRESCALER, true);

	timerAttachInterrupt(fanTimer, &onTimer, true);

	timerAlarmWrite(fanTimer, Config::PERIOD_MS - Config::FAN_ON_TIME_MS, false);

	timerAlarmEnable(fanTimer);
}

void loop()
{
}
