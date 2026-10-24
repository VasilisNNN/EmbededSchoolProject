#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"

extern "C"
{
    volatile uint32_t app_millisecond_ticks = 0U;
}

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

    uint32_t last_tick = app_millisecond_ticks;
    uint32_t random_state = 0xA341316CU;
    uint32_t toggle_interval_ms = 500U;
    uint32_t elapsed_ms = 0U;
    bool button_candidate_pressed = (GPIOA->IDR & button_pin) == 0U;
    bool button_stable_pressed = button_candidate_pressed;
    uint32_t button_candidate_since_ms = last_tick;
    bool led_on = false;

    while (1)
    {
        const uint32_t current_tick = app_millisecond_ticks;
        const uint32_t elapsed_ticks = current_tick - last_tick;

        if (elapsed_ticks == 0U)
        {
            __WFI();
            continue;
        }

        last_tick = current_tick;

        // Read button (active LOW)
        const bool button_pressed = (GPIOA->IDR & button_pin) == 0U;

        // Button debounce
        if (button_pressed != button_candidate_pressed)
        {
            button_candidate_pressed = button_pressed;
            button_candidate_since_ms = current_tick;
        }

        // Button pressed
        if ((button_stable_pressed != button_candidate_pressed) &&
            ((current_tick - button_candidate_since_ms) >= debounce_ms))
        {
            button_stable_pressed = button_candidate_pressed;

            if (button_stable_pressed)
            {
                random_state =
                    next_random_value(random_state ^ current_tick) | 1U;

                const uint32_t interval_range =
                    maximum_toggle_interval_ms -
                    minimum_toggle_interval_ms + 1U;

                toggle_interval_ms =
                    minimum_toggle_interval_ms +
                    (random_state % interval_range);

                elapsed_ms = 0U;

                GPIOC->BSRR = GPIO_BSRR_BR13; // LED OFF

                printf("Button pressed; LED toggle interval: %lu ms\r\n",
                       static_cast<unsigned long>(toggle_interval_ms));
            }
        }

        // LED timing
        elapsed_ms += elapsed_ticks;

        if (elapsed_ms >= toggle_interval_ms)
        {
            const uint32_t toggle_count =
                elapsed_ms / toggle_interval_ms;

            elapsed_ms %= toggle_interval_ms;

            if ((toggle_count & 1U) != 0U)
            {
                led_on = !led_on;

                if (led_on)
                {
                    GPIOC->BSRR = GPIO_BSRR_BR13; // PC13 LOW
                }
                else
                {
                    GPIOC->BSRR = GPIO_BSRR_BS13; // PC13 HIGH
                }
            }
        }
    }
}
