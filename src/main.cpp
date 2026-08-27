#include <Arduino.h>


constexpr uint8_t PIN_IN = 16;
constexpr uint8_t PIN_OUT= 17;



volatile uint32_t interruptTime = 0;

uint32_t startTime = 0;
uint32_t measurement = 0;

uint8_t pinInput = 0;
uint32_t lasttimer = 0;

void IRAM_ATTR onContactChange()
{


    interruptTime = millis();
    
    uint32_t now = millis();
    if (now - lasttimer > 20)
    {
      
        lasttimer = now;
    }

  
    
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_OUT, OUTPUT);
    pinMode(PIN_IN, INPUT_PULLDOWN);

     attachInterrupt(
        digitalPinToInterrupt(PIN_IN),
        onContactChange,
        RISING
    );

    digitalWrite(PIN_OUT, LOW);

    Serial.println("Relay timing test");
}

void loop()
{
    static uint8_t lastpinstate;

     if(lastpinstate==1)
     digitalWrite(PIN_OUT, HIGH);
     else 
     digitalWrite(PIN_OUT, LOW);


    measurement = interruptTime - startTime;

    uint32_t now = millis();

      
    if (now - lasttimer < 2000)
        return;
    
        if(lastpinstate!=1)
        lastpinstate = 1;
        else lastpinstate = 0;

        Serial.printf("interruptTime: %lu\n", interruptTime);
        Serial.printf("read pin: %d\n", digitalRead(PIN_IN));
   
     
    lasttimer = now;
}
