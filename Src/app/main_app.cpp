#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "printf/usb_printf.h"
#include "pwm/pwm.h"
#include "servo/servo.h"
#include "adc/adc.h"

#define SERVO_GPIO_PORT PWM_PORT_B
#define SERVO_GPIO_PIN 4
#define SERVO_STEP_ANGLE 10

extern "C" void main_cpp()
{

    bool error = false;
 
    PwmDriver_t servo_pwm;
    Servo_t servo;

    if (!Pwm_InitByPin(&servo_pwm, SERVO_GPIO_PORT,
                       SERVO_GPIO_PIN, SERVO_FREQUENCY_HZ, 0))
    {
        error = true;
        printf("PWM init failed\n");
    }

    if (!error && !Servo_Init(&servo, &servo_pwm))
    {
        error = true;
        printf("Servo init failed\n");
    }

    const uint32_t adc_channels[] = {ADC_CHANNEL_5};
    uint32_t adc_values[1];

    if (ADC_Init(adc_channels, 1) != HAL_OK)
    {
        Error_Handler();
    }

    float timer = HAL_GetTick();
    while (1)
    {
        if (ADC_ReadSequence(adc_values, 1) != HAL_OK)
            continue;

        uint32_t calculatedAngle = ((adc_values[0] * 180) / ADC_MAX_VALUE)*-1 + 180;

        Servo_SetAngle(&servo, calculatedAngle);

        if (timer < HAL_GetTick())
        {

            printf("SERVO ANGLE: %d\r\n", calculatedAngle);
            timer = HAL_GetTick() + 100;
        }
    }
}
