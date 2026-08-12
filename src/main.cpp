#include <Arduino.h>

#define LED_OUT_RED 15
#define BUTTON_BOOT 0
#define BUTTON_IN 16

u8_t blinkSpeed = 0;
bool legTurnedOn = false;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(BUTTON_BOOT, INPUT_PULLDOWN);
    pinMode(LED_OUT_RED, OUTPUT);
    digitalWrite(LED_OUT_RED, LOW);

    pinMode(BUTTON_IN, INPUT_PULLUP);
}

void loop()
{
    speedManager();
    ledManager();
}

void speedManager()
{
    int input = digitalRead(BUTTON_IN);
    int inputBoot = digitalRead(BUTTON_BOOT);

    if (input == 1)
    {
        blinkSpeed = 1;
        delay(5);
    }
    if (inputBoot == 1)
    {
        blinkSpeed = 2;
        delay(5);
    }
}

void ledManager()
{
    if (blinkSpeed == 0)
    {

        digitalWrite(LED_OUT_RED, LOW);
        Serial.println("LED OFF");

        return;
    }

    legTurnedOn = !legTurnedOn;

    if (legTurnedOn)
    {
        Serial.println("LED ON");
        digitalWrite(LED_OUT_RED, HIGH);
    }
    else
    {
        Serial.println("LED OFF");
        digitalWrite(LED_OUT_RED, LOW);
    }

    delay(400 * blinkSpeed);
}
