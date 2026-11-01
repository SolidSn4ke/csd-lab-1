#ifndef MY_DRIVER_H
#define MY_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include "stm32f4xx_hal.h"

typedef enum
{
    GREEN,
    RED,
    YELLOW
} Lights;

void turn_on_light(Lights color);
void turn_off_light(Lights color);

int check_button(uint32_t now);

typedef enum
{
    INTR,
    POLL
} Modes;

HAL_StatusTypeDef receive_char(Modes mode, char *recieved_char);
typedef struct
{
    size_t index;
    int overflow;
} LineReceiver;

typedef enum
{
    LINE_PENDING,
    LINE_READY,
    LINE_OVERFLOW,
    LINE_ERROR
} LineStatus;

LineStatus receive_str(Modes mode, LineReceiver *rx, char *buf, size_t size);
HAL_StatusTypeDef transmit_char(Modes mode);
HAL_StatusTypeDef transmit_str(Modes mode, char *ptr, size_t size);

#endif
