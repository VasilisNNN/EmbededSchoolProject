#include <Arduino.h>
#include <Config.h>
#include <Task1.h>

 uint32_t count_t1 = 0;
 bool interruptIsActive_t1 = false;


void IRAM_ATTR Task1::onInterrupt_t1()
{
    count_t1++;
    interruptIsActive_t1 = true;
}

void Task1::Init()
{
  

    pinMode(Config::PIN_BUTTON, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(Config::PIN_BUTTON),
        onInterrupt_t1,
        FALLING);
}

void Task1::Update()
{
     Serial.printf(
        "PIN: %d | Count: %lu\n",
        digitalRead(Config::PIN_BUTTON),
        count_t1
    );

    if (interruptIsActive_t1)
    {
        Serial.println("INTERRUPT!");
        interruptIsActive_t1 = false;
    }
}
