#include <Arduino.h>
#include <Config.h>
#include <Task1.h>


    uint32_t Task1::count = 0;
    float timer;

    void IRAM_ATTR Task1::onInterrupt()
    {
        count++;
      
    }


    void Task1::Init()
    {
        attachInterrupt(
            digitalPinToInterrupt(Config::PIN_BUTTON),
            onInterrupt,
            FALLING
        );
    }

    void Task1::Update()
    {
        if(timer > millis())return;

        Serial.printf("Count: %lu\n", count);
        timer =  millis()+300;
        
    }
    