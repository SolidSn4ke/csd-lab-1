#include "../Inc/mydriver.h"
#include "gpio.h"

#define BUTTON_DEBOUNCE_MS 30U

static uint16_t light_pin(Lights color)
{
    switch (color)
    {
    case GREEN:
        return GPIO_PIN_13;

    case YELLOW:
        return GPIO_PIN_14;

    case RED:
        return GPIO_PIN_15;

    default:
        return 0U;
    }
}

void turn_on_light(Lights color)
{
    uint16_t pin = light_pin(color);

    if (pin != 0U)
    {
        HAL_GPIO_WritePin(GPIOD, pin, GPIO_PIN_SET);
    }
}

void turn_off_light(Lights color)
{
    uint16_t pin = light_pin(color);

    if (pin != 0U)
    {
        HAL_GPIO_WritePin(GPIOD, pin, GPIO_PIN_RESET);
    }
}

int check_button(uint32_t now)
{
    static int last_raw = 0;
    static int stable_state = 0;
    static uint32_t changed_at = 0U;

    int raw = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET);

    if (raw != last_raw)
    {
        last_raw = raw;
        changed_at = now;
    }

    if ((uint32_t)(now - changed_at) >= BUTTON_DEBOUNCE_MS)
    {
        stable_state = last_raw;
    }

    return stable_state;
}
