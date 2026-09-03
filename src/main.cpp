#include <Arduino.h>
#include <atomic>
#include <Config.h>

TimerHandle_t fanTimer = nullptr;
bool fanRunning = false;

void fanTimerCallback(TimerHandle_t timer)
{
	if (fanRunning)
	{

		digitalWrite(Config::FAN_PIN, LOW);

		Serial.println("Fan OFF");

		xTimerChangePeriod(
			fanTimer,
			pdMS_TO_TICKS(Config::PERIOD_MS),
			0);

		fanRunning = false;
		return;
	}

	digitalWrite(Config::FAN_PIN, HIGH);

	Serial.println("Fan ON");

	xTimerChangePeriod(
		fanTimer,
		pdMS_TO_TICKS(Config::FAN_ON_TIME_MS),
		0);
	fanRunning = true;
}

void setup()
{
	Serial.begin(115200);

	pinMode(Config::FAN_PIN, OUTPUT);

	digitalWrite(Config::FAN_PIN, LOW);

	fanTimer = xTimerCreate(
		"FanTimer",
		pdMS_TO_TICKS(Config::PERIOD_MS),
		pdFALSE,
		NULL,
		fanTimerCallback);

	if (fanTimer != NULL)
	{
		xTimerStart(fanTimer, 0);
	}
}

void loop()
{
}
