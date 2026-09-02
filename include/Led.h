#include <stdint.h>


enum class LedState
{
    On,
    Off
};

class Led
{
private:
    uint8_t pinID;

public:
    float timer;

    void Init(uint8_t pinId);
    void Set(LedState state);
};
