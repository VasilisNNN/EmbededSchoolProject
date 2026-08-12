#include <Arduino.h>

#define LED_OUT_RED 15
#define BUTTON_BOOT 0

#define BUTTON_IN 16

float ledSpeed = 0;
float prevledSpeed = -1;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(BUTTON_BOOT, INPUT_PULLDOWN);
    pinMode(LED_OUT_RED, OUTPUT);
    digitalWrite(LED_OUT_RED, LOW);

    pinMode(BUTTON_IN, INPUT_PULLDOWN);
}

void loop()
{

    int input = digitalRead(BUTTON_IN);
    int inputBoot = digitalRead(BUTTON_BOOT);
    static bool legTurnedOn = false;

    if (input == 1)
    {
        ledSpeed = 1;
        delay(5);
    }
    if (inputBoot == 1)
    {
        ledSpeed = 2;
        delay(5);
    }

    
    if (ledSpeed == 0)
    {

        digitalWrite(LED_OUT_RED, LOW);

        if (prevledSpeed != ledSpeed)
        {
            Serial.println("LED OFF");
            prevledSpeed = ledSpeed;
        }
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

    delay(100);
}
