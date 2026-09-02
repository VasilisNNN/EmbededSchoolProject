#include <Arduino.h>
#include <Config.h>
#include <Task4.h>


    uint32_t Task4::count = 0;
    uint32_t lastPollTime;

    enum class ButtonState
    {
        Released,
        Pressed
    };
    ButtonState state = ButtonState::Released;

    
    void Task4::Init()
    {
      pinMode(Config::PIN_BUTTON, INPUT_PULLUP);
    }

    void Task4::Update()
    {
       uint32_t now = millis();

    
    if (now - lastPollTime < Config::POLL_INTERVAL)
        return;

    lastPollTime = now;

  
    switch (state)
    {
        case ButtonState::Released:

            if (digitalRead(Config::PIN_BUTTON) == LOW)
            {
                state = ButtonState::Pressed;

                Serial.println("Button pressed");
            }

            break;

        case ButtonState::Pressed:

            if (digitalRead(Config::PIN_BUTTON) == HIGH)
            {
                state = ButtonState::Released;

                Serial.println("Button released");
            }

            break;
    }
    }
