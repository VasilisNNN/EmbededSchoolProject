#include <Arduino.h>
#include <Config.h>
#include <Task3.h>

    uint32_t Task3::count = 0;
    bool Task3::buttonEvent = false;
    

    void IRAM_ATTR Task3::onInterrupt()
    {
        buttonEvent = true;
    }

    void Task3::Init()
    {

        attachInterrupt(
            digitalPinToInterrupt(Config::PIN_BUTTON),
            onInterrupt,
            FALLING);
    }

    void Task3::Update()
    {
        if (!buttonEvent)
            return;

      

        if (digitalRead(Config::PIN_BUTTON) == LOW)
        {
            count++;

            Serial.printf("Button pressed: %lu\n", count);
        }

          buttonEvent = false;
    }
