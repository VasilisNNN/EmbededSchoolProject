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



extern "C" void main_cpp() {



  bool error = false;
  //ADC_HandleTypeDef hadc1;
 // const uint32_t adccount = 1;

 /* if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }*/


    /*if (!Pwm_InitByPin(&pwm_led, PWM_PORT_B, 4, 1000, 0))
    {
        printf("PWM init failed\n");
    }*/


    PwmDriver_t servo_pwm;
    Servo_t servo;

    if (!Pwm_InitByPin(&servo_pwm, SERVO_GPIO_PORT,
                SERVO_GPIO_PIN, SERVO_FREQUENCY_HZ, 0)) {
        error = true;
        printf("PWM init failed\n");
    }

    if (!error && !Servo_Init(&servo, &servo_pwm)) {
        error = true;
        printf("Servo init failed\n");
    }


 



    const uint32_t adc_channels[] = {ADC_CHANNEL_5};
     uint32_t adc_values[1];

    if (ADC_Init(adc_channels, 1) != HAL_OK) {
        Error_Handler();
    }

    while (1) {

 printf("HELLO");
       
  if (ADC_ReadSequence(adc_values, 1) == HAL_OK) {
            printf("ADC PA5: %lu \n",adc_values[0]);
        }


        float calculatedAngle = ((float)adc_values[0] / (float)ADC_MAX_VALUE) *180.0;


              Servo_SetAngle(&servo, calculatedAngle);
        

        HAL_Delay(50);
    }
}
