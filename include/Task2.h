#include <Arduino.h>

class Task2
{
public:
    void Init();
    void Update();
    static void IRAM_ATTR onInterrupt();

private:
    static uint32_t count;
    static bool interruptFlag ;
    static uint32_t interruptTime ;
};
