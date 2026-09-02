#include <Arduino.h>

#include <Arduino.h>

#define PIN_IN 16
#define PIN_OUT 17

bool turnedOn = false;
uint32_t switchTimer = 0;

volatile uint32_t finishTimer = 0;
volatile bool contactChanged = false;

uint32_t startTimer = 0;

#define DEBOUNCE_TIME 100
#define TimderDelay 3000

void IRAM_ATTR onInterrupt()
{
    static uint32_t lastInterruptTime = 0;

    uint32_t currentTime = millis();

    if (currentTime - lastInterruptTime < DEBOUNCE_TIME)
        return;

    lastInterruptTime = currentTime;

    finishTimer = currentTime;
    contactChanged = true;
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_IN, INPUT_PULLDOWN);
    pinMode(PIN_OUT, OUTPUT);

    digitalWrite(PIN_OUT, LOW);

    attachInterrupt(
        digitalPinToInterrupt(PIN_IN),
        onInterrupt,
        RISING);
}

void SwitchOnOff()
{
    if (switchTimer > millis())
        return;

    if (!turnedOn)
    {
        contactChanged = false;
        finishTimer = 0;

        startTimer = millis();

        turnedOn = true;
        digitalWrite(PIN_OUT, HIGH);
    }
    else
    {
        turnedOn = false;

        digitalWrite(PIN_OUT, LOW);
    }

    switchTimer = millis() + TimderDelay;
}

void loop()
{
    SwitchOnOff();

    if (turnedOn && contactChanged)
    {
        uint32_t responseTime = finishTimer - startTimer;

        Serial.printf("StartTime: %lu\n", startTimer);
        Serial.printf("FinishTime: %lu\n", finishTimer);
        Serial.printf("Relay response: %lu ms\n", responseTime);

        contactChanged = false;
    }
}