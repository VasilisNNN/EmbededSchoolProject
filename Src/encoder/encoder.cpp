#include "main.h"

extern TIM_HandleTypeDef htim3;

int32_t encoder_value = 0;

void Encoder_Start(void)
{
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

    __HAL_TIM_SET_COUNTER(&htim3, 0);
}

int32_t Encoder_GetValue(void)
{
    return __HAL_TIM_GET_COUNTER(&htim3);
}