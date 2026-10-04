#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "printf/usb_printf.h"
#include "pwm/pwm.h"
#include "adc/adc.h"
#include "app/main_app.h"
#include "encoder/encoder.h"
#include "screen/ssd1306.h"
#include "fonts/fonts.h"

extern TIM_HandleTypeDef htim3;

int32_t encoder_prev = 0;

int32_t firstOperand = 0;
int32_t secondOperand = 0;
int32_t thirdOperand = 0;

CalculatorOperand calOperator;
MathAction mathAction;
char action[5];
int32_t buttonTimer;
int32_t rotationTimer;

extern "C" void ALT_MAIN_Init()
{
    calOperator = CalculatorOperand::First;
    SSD1306_Init();
}

extern "C" void main_cpp()
{
    action[0] = '+';
    action[1] = '-';
    action[2] = 'x';
    action[3] = '/';
    action[4] = '\0';

    Encoder_Start();
    bool error = false;
    bool reversed = true;

    float result;

    while (1)
    {

        SSD1306_GotoXY(0, 10);

        char text[50];
        char text2[50];
        snprintf(text, sizeof(text), "%ld %c %ld",
                 (long)firstOperand,
                 action[secondOperand],
                 (long)thirdOperand);

        SSD1306_Puts(text, &Font_11x18, SSD1306_COLOR_WHITE);

        SSD1306_GotoXY(0, 40);

        snprintf(text2, sizeof(text2), "Res: %.2f",
                 result);

        SSD1306_Puts(text2, &Font_11x18, SSD1306_COLOR_WHITE);

        SSD1306_UpdateScreen();

        int32_t encoder_now = Encoder_GetValue();
        int32_t diff = encoder_now - encoder_prev;

        int32_t currentTime = HAL_GetTick();

        if (diff != 0 && rotationTimer < currentTime)
        {

            switch (secondOperand)
            {
            case 0:
                mathAction = MathAction::Plus;
                break;
            case 1:
                mathAction = MathAction::Minus;
                break;
            case 2:
                mathAction = MathAction::Multiply;
                break;
            case 3:
                mathAction = MathAction::Divide;
                break;

            default:
                break;
            }

            if (diff > 0)
            {
                SSD1306_Clear();

                printf("TURN RIGHT \n");

                if (calOperator == CalculatorOperand::First)
                    firstOperand++;

                if (calOperator == CalculatorOperand::Second)
                    if (rotationTimer < currentTime)
                    {
                        if (secondOperand < strlen(action) - 1)
                            secondOperand++;
                        rotationTimer = currentTime + 500;
                    }
                if (calOperator == CalculatorOperand::Third)
                    thirdOperand++;
            }
            else if (diff < 0)
            {
                SSD1306_Clear();

                printf("TURN LEFT \n");

                if (calOperator == CalculatorOperand::First)
                    firstOperand--;

                if (calOperator == CalculatorOperand::Second)
                {
                    if (rotationTimer < currentTime)
                    {
                        if (secondOperand > 0)
                            secondOperand--;
                        rotationTimer = currentTime + 500;
                    }
                }
                if (calOperator == CalculatorOperand::Third)
                    thirdOperand--;
            }

            if (calOperator != CalculatorOperand::Second)
                rotationTimer = currentTime + 100;
            encoder_prev = encoder_now;
        }

        static uint32_t calculatedAngle = 90;
        calculatedAngle++;

        if (calculatedAngle > 180)
            calculatedAngle = 0;

        static GPIO_PinState button;
        button = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);

        result = CalculateResult(&mathAction);

        if (button == 0)
        {
            if (buttonTimer < currentTime)
            {

                switch (calOperator)
                {
                case CalculatorOperand::First:
                    calOperator = CalculatorOperand::Second;
                    break;
                case CalculatorOperand::Second:
                    calOperator = CalculatorOperand::Third;
                    break;
                case CalculatorOperand::Third:
                    calOperator = CalculatorOperand::First;
                    break;

                default:
                    break;
                }

                printf("Programmer button pressed");

                buttonTimer = currentTime + 1000;
            }
        }
    }
}

float CalculateResult(MathAction *cop)
{
    switch (*cop)
    {
    case MathAction::Plus:
        return firstOperand + thirdOperand;

    case MathAction::Minus:
        return firstOperand - thirdOperand;

    case MathAction::Multiply:
        return firstOperand * thirdOperand;

    case MathAction::Divide:
        return (float)firstOperand / (float)thirdOperand;

    default:
        return 0.0f;
    }
}