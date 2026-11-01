#include "../Inc/mydriver.h"
#include "gpio.h"
#include "my_gpio.h"
#include <usart.h>

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
        My_GPIO_WritePins(GPIOD, pin, pin);
    }
}

void turn_off_light(Lights color)
{
    uint16_t pin = light_pin(color);

    if (pin != 0U)
    {
        My_GPIO_WritePins(GPIOD, pin, 0U);
    }
}

int check_button(uint32_t now)
{
    static int last_raw = 0;
    static int stable_state = 0;
    static uint32_t changed_at = 0U;

    int raw = (My_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET);

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

HAL_StatusTypeDef receive_char(Modes mode, char *recieved_char)
{
    if (recieved_char == NULL)
    {
        return HAL_ERROR;
    }

    switch (mode)
    {
    case INTR:
        return HAL_ERROR;

    case POLL:
        return HAL_UART_Receive(&huart6, (uint8_t *)recieved_char, 1, 0);

    default:
        return HAL_ERROR;
    }
}

LineStatus receive_str(Modes mode, LineReceiver *rx, char *buf, size_t size)
{
    if (rx == NULL || buf == NULL || size == 0 || rx->index >= size)
        return LINE_ERROR;

    char ch;
    HAL_StatusTypeDef status = receive_char(mode, &ch);

    if (status == HAL_TIMEOUT || status == HAL_BUSY)
        return LINE_PENDING;

    if (status != HAL_OK)
        return LINE_ERROR;

    if (ch == '\r' || ch == '\n')
    {
        if (rx->overflow)
        {
            rx->index = 0;
            rx->overflow = 0;
            buf[0] = '\0';
            return LINE_OVERFLOW;
        }

        if (rx->index == 0)
            return LINE_PENDING;

        buf[rx->index] = '\0';
        rx->index = 0;
        return LINE_READY;
    }

    if (rx->overflow)
        return LINE_PENDING;

    if (rx->index >= size - 1)
    {
        rx->overflow = 1;
        return LINE_PENDING;
    }

    buf[rx->index++] = ch;
    buf[rx->index] = '\0';
    return LINE_PENDING;
}
