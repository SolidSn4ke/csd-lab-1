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

#ifndef UART_RX_QUEUE_SIZE
#define UART_RX_QUEUE_SIZE 256U
#endif

#if UART_RX_QUEUE_SIZE < 2
#error "UART_RX_QUEUE_SIZE must be at least 2"
#endif

/* Call after MX_USART6_UART_Init, from main only. No DMA is used.
 * Reinitialization discards queued bytes: also reset the caller's LineReceiver.
 * Do not call while a UART transmission is active.
 */
HAL_StatusTypeDef uart_receive_init(Modes mode);

/* mode must match uart_receive_init. HAL_BUSY means the RX queue is empty. */
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
