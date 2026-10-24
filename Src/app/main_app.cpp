#include "stm32f4xx.h"

namespace
{
constexpr uint32_t led_pin = 1U << 13U;
constexpr uint32_t button_pin = 1U << 0U;
constexpr uint32_t debounce_ms = 25U;
constexpr uint32_t minimum_toggle_interval_ms = 200U;
constexpr uint32_t maximum_toggle_interval_ms = 2000U;

void configure_gpio()
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;
    (void)RCC->AHB1ENR;

    GPIOC->MODER = (GPIOC->MODER & ~(3U << (13U * 2U))) |
                   (1U << (13U * 2U));
    GPIOC->OTYPER &= ~led_pin;
    GPIOC->OSPEEDR &= ~(3U << (13U * 2U));
    GPIOC->PUPDR &= ~(3U << (13U * 2U));
    GPIOC->BSRR = led_pin;

    GPIOA->MODER &= ~(3U << (0U * 2U));
    GPIOA->PUPDR = (GPIOA->PUPDR & ~(3U << (0U * 2U))) |
                   (1U << (0U * 2U));
}

void configure_systick()
{
    SysTick->CTRL = 0U;
    SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

void wait_for_systick_tick()
{
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
    {
        __NOP();
    }
}

uint32_t next_random_value(uint32_t value)
{
    value ^= value << 13U;
    value ^= value >> 17U;
    value ^= value << 5U;
    return value;
}
}

extern "C" void main_cpp(void)
{
    configure_gpio();
    configure_systick();

    uint32_t random_state = 0xA341316CU;
    uint32_t toggle_interval_ms = 500U;
    uint32_t elapsed_ms = 0U;
    uint32_t button_stable_ms = 0U;
    bool button_candidate_pressed = (GPIOA->IDR & button_pin) == 0U;
    bool button_stable_pressed = button_candidate_pressed;
    bool led_on = false;

    while (1)
    {
        wait_for_systick_tick();
        random_state = next_random_value(random_state);

        const bool button_pressed = (GPIOA->IDR & button_pin) == 0U;
        if (button_pressed != button_candidate_pressed)
        {
            button_candidate_pressed = button_pressed;
            button_stable_ms = 0U;
        }
        else if (button_stable_ms < debounce_ms)
        {
            ++button_stable_ms;
        }

        if ((button_stable_ms >= debounce_ms) &&
            (button_stable_pressed != button_candidate_pressed))
        {
            button_stable_pressed = button_candidate_pressed;
            if (button_stable_pressed)
            {
                const uint32_t interval_range =
                    maximum_toggle_interval_ms - minimum_toggle_interval_ms + 1U;
                toggle_interval_ms = minimum_toggle_interval_ms +
                                     (random_state % interval_range);
                elapsed_ms = 0U;
            }
        }

        ++elapsed_ms;
        if (elapsed_ms >= toggle_interval_ms)
        {
            elapsed_ms = 0U;
            led_on = !led_on;
            GPIOC->BSRR = led_on ? (led_pin << 16U) : led_pin;
        }
    }
}
