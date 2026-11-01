#ifndef BYTE_QUEUE_H
#define BYTE_QUEUE_H

#include <stddef.h>
#include <stdint.h>

/* The caller serializes access. One slot is reserved to detect a full queue. */
typedef struct
{
    uint8_t *data;
    size_t size;
    size_t head;
    size_t tail;
} ByteQueue;

static inline void byte_queue_clear(ByteQueue *queue)
{
    queue->head = 0;
    queue->tail = 0;
}

static inline int byte_queue_push(ByteQueue *queue, uint8_t byte)
{
    size_t next = (queue->head + 1U) % queue->size;
    if (next == queue->tail)
        return 0;

    queue->data[queue->head] = byte;
    queue->head = next;
    return 1;
}

static inline int byte_queue_pop(ByteQueue *queue, uint8_t *byte)
{
    if (queue->head == queue->tail)
        return 0;

    *byte = queue->data[queue->tail];
    queue->tail = (queue->tail + 1U) % queue->size;
    return 1;
}

#endif
