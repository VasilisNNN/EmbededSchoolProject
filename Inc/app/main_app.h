#include "screen/ssd1306.h"

enum CalculatorOperand
{
    First,
    Second,
    Third,
    Four
};

enum MathAction
{
    Plus,
    Minus,
    Divide,
    Multiply
};


float CalculateResult(MathAction *cop);