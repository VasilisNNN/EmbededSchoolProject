#include <Arduino.h>


#define PIN_IN 16
#define ADCmax 4095

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_IN, INPUT);


}

void loop()
{
  int ADCValue = digitalRead(PIN_IN);
 int AnalogValue = analogRead(PIN_IN);
  int AnalogVoltage = analogReadMilliVolts(PIN_IN);

 const float URef = 3100;

 float Voltage = ((float)AnalogValue/(float)4095) * URef;

 float error = abs(Voltage - AnalogVoltage) / AnalogVoltage * 100.0;

 Serial.printf("digitalRead: %d \n", ADCValue);
 Serial.printf("AnalogValue: %d mV\n", AnalogValue);
 Serial.printf("Voltage: %.02f V\n", Voltage);
 Serial.printf("AnalogValueM: %.02f mV\n", AnalogVoltage);
 Serial.printf("error: %.02f mV\n", error);

    delay(100);
}
