#include "../Inc/mydriver.h"
#include "gpio.h"
#include "my_gpio.h"
#include <usart.h>
#include "../Inc/byte_queue.h"

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

static uint8_t rx_storage[UART_RX_QUEUE_SIZE];
static ByteQueue rx_queue = {rx_storage, sizeof(rx_storage), 0, 0};
static uint8_t rx_byte;
static volatile Modes rx_mode = POLL;
static volatile int rx_initialized;
static volatile int rx_fault;
static volatile int rx_restart_needed;

static uint8_t tx_storage[UART_TX_QUEUE_SIZE];
static ByteQueue tx_queue = {tx_storage, sizeof(tx_storage), 0, 0};
static uint8_t tx_byte;
static volatile int tx_active;

/* Keep the queue's tail byte until HAL reports transmission complete.
 * Called with interrupts masked, or from the USART6 handler.
 */
static HAL_StatusTypeDef tx_start(void)
{
    if (tx_active || tx_queue.head == tx_queue.tail)
        return HAL_OK;

    tx_byte = tx_queue.data[tx_queue.tail];
    HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&huart6, &tx_byte, 1);
    if (status == HAL_OK)
        tx_active = 1;
    return status;
}

HAL_StatusTypeDef uart_transmit_init(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    HAL_StatusTypeDef status = HAL_OK;
    if (!rx_initialized)
        status = HAL_ERROR;
    else if (rx_mode == INTR)
        status = tx_start();
    __set_PRIMASK(primask);
    return status;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != &huart6 || !tx_active)
        return;

    uint8_t completed;
    (void)byte_queue_pop(&tx_queue, &completed);
    tx_active = 0;
    /* A failed start retains queued bytes; main retries through service. */
    (void)tx_start();
}

/* Called with interrupts masked, or from the USART6 handler. */
static HAL_StatusTypeDef rx_start(void)
{
    HAL_StatusTypeDef status = HAL_UART_Receive_IT(&huart6, &rx_byte, 1);
    rx_restart_needed = (status != HAL_OK);
    if (status != HAL_OK)
        rx_fault = 1;
    return status;
}

HAL_StatusTypeDef uart_receive_init(Modes mode)
{
    if (mode != INTR && mode != POLL)
        return HAL_ERROR;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (huart6.gState != HAL_UART_STATE_READY || tx_active ||
        tx_queue.head != tx_queue.tail)
    {
        __set_PRIMASK(primask);
        return HAL_BUSY;
    }

    HAL_NVIC_DisableIRQ(USART6_IRQn);
    HAL_StatusTypeDef status = HAL_UART_AbortReceive(&huart6);
    int was_initialized = rx_initialized;
    rx_initialized = 0;

    if (status == HAL_OK)
    {
        if (!was_initialized)
        {
            __HAL_UART_CLEAR_OREFLAG(&huart6);
            byte_queue_clear(&rx_queue);
            rx_fault = 0;
        }
        else if (__HAL_UART_GET_FLAG(&huart6, UART_FLAG_ORE))
        {
            __HAL_UART_CLEAR_OREFLAG(&huart6);
            rx_fault = 1;
        }
        else if (__HAL_UART_GET_FLAG(&huart6, UART_FLAG_RXNE))
        {
            /* Preserve a byte received just before the mode change. */
            uint8_t byte;
            if (HAL_UART_Receive(&huart6, &byte, 1, 0) != HAL_OK ||
                !byte_queue_push(&rx_queue, byte))
                rx_fault = 1;
        }
        HAL_NVIC_ClearPendingIRQ(USART6_IRQn);
        rx_restart_needed = 0;
        rx_mode = mode;

        if (mode == INTR)
            status = rx_start();

        if (status == HAL_OK)
        {
            rx_initialized = 1;
            if (mode == INTR)
                HAL_NVIC_EnableIRQ(USART6_IRQn);
        }
    }

    __set_PRIMASK(primask);
    return status;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != &huart6 || !rx_initialized || rx_mode != INTR)
        return;

    /* Ignore bytes after a detected loss until main acknowledges the fault. */
    if (!rx_fault && !byte_queue_push(&rx_queue, rx_byte))
        rx_fault = 1;

    (void)rx_start();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart != &huart6 || !rx_initialized || rx_mode != INTR)
        return;

    rx_fault = 1;
    rx_restart_needed = 1;
    /* HAL can stop reception on ORE. Abort also handles recoverable errors. */
    if (HAL_UART_AbortReceive(huart) == HAL_OK)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
        (void)rx_start();
    }
}

HAL_StatusTypeDef receive_char(Modes mode, char *recieved_char)
{
    if (recieved_char == NULL)
    {
        return HAL_ERROR;
    }

    if (!rx_initialized || mode != rx_mode)
        return HAL_ERROR;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (mode == INTR && rx_restart_needed)
    {
        if (HAL_UART_AbortReceive(&huart6) == HAL_OK)
        {
            __HAL_UART_CLEAR_OREFLAG(&huart6);
            (void)rx_start();
        }
    }

    HAL_StatusTypeDef status;
    uint8_t byte;
    if (rx_fault)
    {
        byte_queue_clear(&rx_queue);
        rx_fault = 0;
        status = HAL_ERROR;
    }
    else
    {
        status = byte_queue_pop(&rx_queue, &byte) ? HAL_OK : HAL_BUSY;
        if (status == HAL_OK)
            *recieved_char = (char)byte;
    }
    __set_PRIMASK(primask);

    if (mode == POLL && status == HAL_BUSY)
        return HAL_UART_Receive(&huart6, (uint8_t *)recieved_char, 1, 0);
    return status;
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
    {
        /* A lost byte must not turn a damaged line into a valid command. */
        rx->index = 0;
        rx->overflow = 1;
        buf[0] = '\0';
        return LINE_ERROR;
    }

    return receive_line_char(rx, buf, size, ch);
}

LineStatus receive_line_char(LineReceiver *rx, char *buf, size_t size, char ch)
{
    if (rx == NULL || buf == NULL || size == 0 || rx->index >= size)
        return LINE_ERROR;

    if (ch == '\0')
    {
        rx->overflow = 1;
        return LINE_PENDING;
    }

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

HAL_StatusTypeDef transmit_char(Modes mode, char ch)
{
    return transmit_str(mode, &ch, 1);
}

HAL_StatusTypeDef transmit_str(Modes mode, const char *ptr, size_t size)
{
    if (!rx_initialized || mode != rx_mode || ptr == NULL || size == 0)
        return HAL_ERROR;

    switch (mode)
    {
    case INTR:
    {
        if (size > UART_TX_QUEUE_SIZE - 1U)
            return HAL_BUSY;

        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        size_t used = (tx_queue.head + tx_queue.size - tx_queue.tail) %
                      tx_queue.size;
        if (size > tx_queue.size - 1U - used)
        {
            __set_PRIMASK(primask);
            return HAL_BUSY;
        }

        size_t old_head = tx_queue.head;
        for (size_t i = 0; i < size; ++i)
            (void)byte_queue_push(&tx_queue, (uint8_t)ptr[i]);

        HAL_StatusTypeDef status = tx_start();
        if (status != HAL_OK)
            tx_queue.head = old_head;

        __set_PRIMASK(primask);
        return status;
    }

    case POLL:
        if (size > UINT16_MAX)
            return HAL_ERROR;
        return HAL_UART_Transmit(&huart6, (uint8_t *)ptr, (uint16_t)size,
                                 HAL_MAX_DELAY);

    default:
        return HAL_ERROR;
    }
}
