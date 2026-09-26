#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "printf/usb_printf.h"
#include "pwm/pwm.h"
#include "servo/servo.h"


#define SERVO_GPIO_PORT PWM_PORT_B
#define SERVO_GPIO_PIN 4
#define SERVO_STEP_ANGLE 10



extern "C" void main_cpp() {
  bool error = false;
  ADC_HandleTypeDef hadc1;

  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

    const uint32_t adcpin = 1;
    const uint32_t adcref = 4095;

   /* if (!Pwm_InitByPin(&pwm_led, PWM_PORT_B, 4, 1000, 0))
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

    while (1) {

       uint32_t adcValue = HAL_ADC_GetValue(&hadc1);
       printf("ADC %d",adcValue);


        if (error) {
            printf("Init error\n");
        } else {
            for (uint16_t angle = SERVO_MIN_ANGLE;
                 angle <= SERVO_MAX_ANGLE; angle += SERVO_STEP_ANGLE) {
                Servo_SetAngle(&servo, angle);
                HAL_Delay(100);
            }

            for (int angle = SERVO_MAX_ANGLE; angle >= SERVO_MIN_ANGLE;
                 angle -= SERVO_STEP_ANGLE) {
                Servo_SetAngle(&servo, (uint16_t)angle);
                HAL_Delay(100);
            }
        }

        HAL_Delay(500);
    }
}
