#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "printf/usb_printf.h"
#include "pwm/pwm.h"
#include "adc/adc.h"



extern "C" void main_cpp()
{
    PwmDriver_t pwm_led;
    const uint32_t adcpin = 1;
    const uint32_t adcref = 4095;

    if (!Pwm_InitByPin(&pwm_led, PWM_PORT_B, 4, 1000, 0))
    {
        printf("PWM init failed\n");
    }

  
    ADC_Init(&adcpin,1);

    float duty = 0;
    int32_t step = 10;
    uint32_t adcvalue = 0;
    while (1)
    {
        /*  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
          HAL_Delay(50);
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
          HAL_Delay(50);
         */

        ADC_ReadSequence(&adcvalue,1);


         duty = ((float)adcvalue / (float)adcref) * 100.0;

        
        Pwm_SetDutyPercent(&pwm_led, (uint32_t)duty);
        Pwm_SetDutyPercent(&pwm_led, (uint32_t)duty);
        printf("PWM duty = %ld%%\n ADC value = %ld\n", (long)duty, adcvalue);



        HAL_Delay(100);
    }
}
