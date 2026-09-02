#include <Arduino.h>

class Task3
{
public:
    void Init();
    void Update();
    static void IRAM_ATTR onInterrupt();

private:
    static uint32_t count;
    static bool buttonEvent ;

};