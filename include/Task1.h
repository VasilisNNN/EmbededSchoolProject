#include <Arduino.h>

class Task1
{
public:
    void Init();
    void Update();
    static void IRAM_ATTR onInterrupt();
    void ChangeCount(uint32_t add);
    uint32_t GetCount();

private:
      static uint32_t count;
};
