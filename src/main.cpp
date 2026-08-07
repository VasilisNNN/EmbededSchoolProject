#include <Arduino.h>

#define LED_OUT_RED 15
#define LED_OUT_BLUE 16

bool ledState = false;

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_OUT_RED, OUTPUT);
    digitalWrite(LED_OUT_RED, LOW);

    pinMode(LED_OUT_BLUE, OUTPUT);
    digitalWrite(LED_OUT_BLUE, LOW);
}

void loop() {
    if (ledState) {
        digitalWrite(LED_OUT_RED, LOW);
        digitalWrite(LED_OUT_BLUE, HIGH);
        ledState = false;
        Serial.println("LED OFF");
    } else {
        digitalWrite(LED_OUT_RED, HIGH);
        digitalWrite(LED_OUT_BLUE, LOW);
        ledState = true;
        Serial.println("LED ON");
    }

    delay(1000);
}
