#include <Arduino.h>

class Task1
{
public:
    void Init();
    void Update();
    
    static void IRAM_ATTR onInterrupt_t1();

};
