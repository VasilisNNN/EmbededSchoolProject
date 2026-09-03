#include <Arduino.h>
#include <Config.h>
#include <Task1.h>
#include <Task2.h>
#include <Task3.h>
#include <Task4.h>
#include "main.h"

/*Task1 task1;
Task2 task2;
Task3 task3;
Task4 task4;*/

uint32_t count_main_t1 = 0;
uint32_t count_main_t2 = 0;
uint32_t count_main_t3 = 0;
uint32_t count_main_t4 = 0;

uint32_t lastInterruptTime = 0;
bool t4printed;

enum class ButtonState
{
	RELEASED,
	PRESSED
};

bool interruptIsActive_main = false;
void IRAM_ATTR onInterrupt_main_t1()
{
	// if(!interruptIsActive_main)count_main = 0;
	count_main_t1++;
	count_main_t2++;
	count_main_t3++;
	count_main_t4++;
	interruptIsActive_main = true;
}

void setup()
{

	Serial.begin(115200);

	pinMode(Config::PIN_BUTTON, INPUT_PULLUP);

	attachInterrupt(
		digitalPinToInterrupt(Config::PIN_BUTTON),
		onInterrupt_main_t1,
		FALLING);

	// task1.Init_t1();
	// task2.Init();
	// task3.Init();
	// task4.Init();
}

void loop()
{
	if (digitalRead(Config::PIN_BUTTON) == HIGH)
	{
		count_main_t1 = 0;
		count_main_t2 = 0;
		count_main_t3 = 0;
		count_main_t4 = 0;

		return;
	}

	//task1();
	//task2();
	task3();
	//task4();

	// task1.Update_t1();
	// task2.Update();
	// task3.Update();
	// task4.Update();
}

void task1()
{
	if (interruptIsActive_main)
	{
		Serial.printf(
			"Count task1: %lu\n",
			count_main_t1);

		// count_main = 0;
		interruptIsActive_main = false;
	}
}

void task2()
{
	if (!interruptIsActive_main)
		return;

	interruptIsActive_main = false;

	uint32_t currentTime = millis();

	if (currentTime - lastInterruptTime >= Config::DEBOUNCE_TIME)
	{
		count_main_t2++;
		lastInterruptTime = currentTime;

		Serial.printf("Count task2: %lu\n", count_main_t2);
	}
}

void task3()
{
	if (!interruptIsActive_main)
		return;

	interruptIsActive_main = false;

	if (digitalRead(Config::PIN_BUTTON) == LOW)
	{
		count_main_t3++;

		Serial.printf("Count task3: %lu\n", count_main_t3);
	}
}

void task4()
{
	static ButtonState state = ButtonState::RELEASED;
	static uint32_t lastPollTime = 0;

	uint32_t currentTime = millis();

	if (digitalRead(Config::PIN_BUTTON) == LOW)
		state = ButtonState::PRESSED;

	if (digitalRead(Config::PIN_BUTTON) == HIGH)
	{
		t4printed = false;
		Serial.printf("RELEASED");
		state = ButtonState::RELEASED;
	}

	if (currentTime - lastPollTime < 10)
		return;

	switch (state)
	{
	case ButtonState::RELEASED:
	{

		break;
	}

	case ButtonState::PRESSED:
	{
		count_main_t4++;
		
		if (!t4printed)
		{
			Serial.printf("PRESSED");
			Serial.printf("Count task4: %lu\n", count_main_t4);
			t4printed = true;
		}
		break;
	}
	}

	lastPollTime = currentTime;
}
