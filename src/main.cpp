#include <Arduino.h>

constexpr uint8_t RELAY_PIN = 17;
constexpr uint8_t CONTACT_PIN = 16;

volatile uint32_t interruptTime = 0;

uint32_t startTime = 0;
uint32_t measurement = 0;

uint8_t pinInput = 0;

void IRAM_ATTR onContactChange()
{
    interruptTime = millis();
  
}

void setup()
{
    Serial.begin(115200);

    pinMode(RELAY_PIN, OUTPUT);
    pinMode(CONTACT_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(CONTACT_PIN),
        onContactChange,
        CHANGE
    );

    digitalWrite(RELAY_PIN, LOW);

    Serial.println("Relay timing test");
}

void loop()
{

    digitalWrite(RELAY_PIN, HIGH);

    measurement = interruptTime - startTime;

    if(interruptTime > 0)
    {
     Serial.printf("interruptTime: ",interruptTime + "\n");
    }
   
}

