#include <Arduino.h>

class GeneralTask
{
public:
    uint32_t count;
    void IRAM_ATTR onInterrupt();
    virtual void Init();
    virtual void Update();
};
