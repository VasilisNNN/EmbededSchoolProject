#include <Arduino.h>

#define PIN_IN 16
#define PIN_OUT 17

#define ADCres 4095
bool turnedOn = false;
float switchTimer = 0;

#define MinSwitchmV 80.0f
#define MaxSwitchmV 120.0f
#define TimderDelay 2.0f

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_IN, INPUT_PULLUP);
    pinMode(PIN_OUT, OUTPUT);
}
void SwitchOnOff(float voltage)
{
  
    if (switchTimer > millis())
        return;

    if (voltage < MinSwitchmV)
    {
        turnedOn = true;

    }
    else if (voltage > MaxSwitchmV)
    {
        turnedOn = false;

    }

    switchTimer = millis() + TimderDelay;
}

void loop()
{
    int ADCValue = digitalRead(PIN_IN);
    int AnalogValue = analogRead(PIN_IN);
    int AnalogVoltage = analogReadMilliVolts(PIN_IN);

    const float URef = 3100;

    float Voltage = ((float)AnalogValue / (float)ADCres) * URef;

    SwitchOnOff(Voltage);

    if (turnedOn)
        digitalWrite(PIN_OUT, HIGH);
    else
        digitalWrite(PIN_OUT, LOW);

    Serial.printf("digitalRead: %d \n", ADCValue);
    Serial.printf("AnalogValue: %d mV\n", AnalogValue);
    Serial.printf("Voltage: %.02f mV\n", Voltage);
    Serial.printf("AnalogValueM: %.02f mV\n", AnalogVoltage);

    delay(100);
}


