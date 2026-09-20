#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "printf/usb_printf.h"
#include "pwm/pwm.h"
#include "sound/sound.h"

const uint32_t c_note = 261;
const uint32_t d_note = 294;
const uint32_t e_note = 330;

const uint32_t SOUND_FREQUENCY_HZ[] = {
    c_note,
    c_note,
    c_note,
    d_note,
    e_note,
    e_note,
    d_note,
};

extern "C" void main_cpp()
{
    PwmDriver_t pwm_led;
    static uint16_t currentNote = 0;
    uint32_t pwm_frequency_hz = Sound_GetPwmFrequency(SOUND_FREQUENCY_HZ[currentNote]);

    static uint32_t timer = 0;

    if (!Pwm_InitByPin(&pwm_led, PWM_PORT_B, 4, pwm_frequency_hz, 50))
    {
        printf("PWM init failed\n");
    }

    if (!Sound_Init(&pwm_led, SOUND_FREQUENCY_HZ[currentNote]))
    {
        printf("Sound init failed\n");
    }

    timer = HAL_GetTick() + 200;
    const uint32_t noteCount =
        sizeof(SOUND_FREQUENCY_HZ) / sizeof(SOUND_FREQUENCY_HZ[0]);

     while (1)
    {
        uint32_t now = HAL_GetTick();

        if (now >= timer)
        {
            currentNote++;

            if (currentNote >= noteCount)
            {
                currentNote = 0;
            }

            uint32_t frequency =
                SOUND_FREQUENCY_HZ[currentNote];

           
            const uint32_t timerFrequency = 1000000;

            uint32_t period = (timerFrequency / frequency) - 1;

           
            __HAL_TIM_SET_AUTORELOAD(
                &pwm_led.htim,
                period
            );

            timer = now + 200;
        }
    }
}
