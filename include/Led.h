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
    void Init();
    void Set(LedState state);

};
