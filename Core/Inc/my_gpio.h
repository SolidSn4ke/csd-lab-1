#ifndef MY_GPIO_H
#define MY_GPIO_H

#include "stm32f4xx.h"

typedef struct
{
  uint16_t PinMask;
  uint32_t Mode;
  uint32_t OType;
  uint32_t Speed;
  uint32_t Pull;
  uint32_t Alternate;
  uint8_t InitialState;
} My_GPIO_InitTypeDef;

/*
 * Mode: младшие два бита MODER; OType задаётся отдельно (0: PP, 1: OD).
 * При повторной настройке GPIO-выхода каскад остаётся включённым,
 * порядок изменения типа, подтяжки и ODR зависит от конечного состояния.
 * InitialState != 0 для open-drain отпускает линию: напряжение зависит
 * от внешней схемы и подтяжек. InitialState применяется только к GPIO-выходу.
 *
 * Перед переходом из/в AF вызывающий код должен остановить старый источник
 * сигнала, подготовить новую периферию и обеспечить требуемый уровень
 * внешней схемой на время промежуточного режима входа.
 * Одновременная перенастройка одного порта из другого контекста не допускается.
 */
void My_GPIO_Init(GPIO_TypeDef *GPIOx, const My_GPIO_InitTypeDef *InitStr);

void My_GPIO_GetConfig(GPIO_TypeDef *GPIOx, uint16_t Pin,
                       My_GPIO_InitTypeDef *InitStr);

uint16_t My_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t PinMask);
uint16_t My_GPIO_ReadPort(GPIO_TypeDef *GPIOx);

void My_GPIO_WritePins(GPIO_TypeDef *GPIOx, uint16_t PinMask,
                       uint16_t StateMask);

#endif
