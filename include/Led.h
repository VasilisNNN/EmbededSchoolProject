#include <stdint.h>
#include <main.h>
#include <Arduino.h>

enum class LedState
{
    On,
    Off
};

class Led
{
private:
    u_int8_t pinID;

public:
    void Init(u_int8_t pinId);
    void Set(LedState state);
};
