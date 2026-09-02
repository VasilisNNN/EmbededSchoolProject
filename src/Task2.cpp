#include <Arduino.h>
#include <Config.h>
#include <Task2.h>


    uint32_t Task2::count = 0;
    bool Task2::interruptFlag = false;
    uint32_t Task2::interruptTime = 0;

     void IRAM_ATTR Task2::onInterrupt()
    {
        count++;
     
        interruptTime = millis();
        interruptFlag = true;
    }

    void Task2::Init()
    {

        attachInterrupt(
            digitalPinToInterrupt(Config::PIN_BUTTON),
            onInterrupt,
            FALLING);
    }

    void Task2::Update()
    {
        static uint32_t lastInterruptTime = 0;

        if (interruptFlag)
        {
           
            uint32_t currentTime = interruptTime;
    

            if (currentTime - lastInterruptTime >= Config::DEBOUNCE_TIME)
            {
                lastInterruptTime = currentTime;

                Serial.println("Interrupt!");
            }

             interruptFlag = false;
        }

        
    }
