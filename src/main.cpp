#include <Arduino.h>
#include <Config.h>
#include <Task1.h>
#include <Task2.h>
#include <Task3.h>
#include <Task4.h>


Task1 task1;
Task2 task2;
Task3 task3;
Task4 task4;



void setup() 
{
    Serial.begin(115200);

   // task1.Init();
    task2.Init();
   // task3.Init();
   // task4.Init();




}

void loop() 
{
   // task1.Update();
    task2.Update();
   // task3.Update();
   // task4.Update();
  


}
