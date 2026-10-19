#ifndef MY_DRIVER_H
#define MY_DRIVER_H

#include <stdint.h>

typedef enum
{
    GREEN,
    RED,
    YELLOW
} Lights;

void turn_on_light(Lights color);
void turn_off_light(Lights color);

/*
 * Вызывать регулярно.
 * Возвращает 1, пока кнопка нажата, после подавления дребезга.
 * now — текущее значение HAL_GetTick().
 */
int check_button(uint32_t now);

#endif
