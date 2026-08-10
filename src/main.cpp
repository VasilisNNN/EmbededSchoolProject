#include <Arduino.h>

#define LED_OUT_RED 15
#define BUTTON_IN 16

bool ledState = false;
bool prevledState = true;

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_OUT_RED, OUTPUT);
    digitalWrite(LED_OUT_RED, LOW);

    pinMode(BUTTON_IN, INPUT_PULLDOWN);
  
}

void loop() {

    int input = digitalRead(BUTTON_IN);
     
      if (input == 1)ledState = true;
      else ledState = false;


    if (ledState) {
        digitalWrite(LED_OUT_RED, HIGH);

        if(prevledState!=ledState)
        {
        Serial.println("LED ON");
        prevledState = ledState;
        }


    } else {
        digitalWrite(LED_OUT_RED, LOW);

      
        if(prevledState!=ledState)
        {
        Serial.println("LED OFF");
        prevledState = ledState;
        }
    }
    

 
}
