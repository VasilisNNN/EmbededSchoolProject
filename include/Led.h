#include <stdint.h>
#include <main.h>

enum class LedState
{
    On,
    Off
};

class Led
{
public:
    void Init(u_int8_t);
    void Set(LedState state);

};
