#include <stm32f427xx.h>
#ifndef MY_DRIVER
#define MY_DRiVER

typedef enum {
    GREEN, RED, YELLOW
} Lights;

void turn_on_light(Lights color);

void turn_off_light(Lights color);

int check_button();

#endif
