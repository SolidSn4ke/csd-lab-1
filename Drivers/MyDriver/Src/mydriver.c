#include "gpio.h"
#include <stm32f427xx.h>
#include "../Inc/mydriver.h"

int check_pin(uint16_t pin) {
    return HAL_GPIO_ReadPin(GPIOD, pin);
}

void switch_light(Lights light) {
    switch (light)
    {
    case GREEN:
        if (!check_pin(GPIO_PIN_13))
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
        break;

    case YELLOW:
        if (!check_pin(GPIO_PIN_14))
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
        break;

    case RED:
        if (!check_pin(GPIO_PIN_15))
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        else HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
        break;
        
    default:
        break;
    }
}
