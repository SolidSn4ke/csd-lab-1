#include "gpio.h"
#include <stm32f427xx.h>
#include "../Inc/mydriver.h"

int check_pin(uint16_t pin)
{
    return HAL_GPIO_ReadPin(GPIOD, pin);
}

void turn_off_light(Lights light)
{
    switch (light)
    {
    case GREEN:
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
        break;

    case YELLOW:
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
        break;

    case RED:
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
        break;

    default:
        break;
    }
}

void turn_on_light(Lights light)
{
    switch (light)
    {
    case GREEN:
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        break;

    case YELLOW:
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        break;

    case RED:
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        break;

    default:
        break;
    }
}

int check_button()
{
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
}
