#include "my_gpio.h"

void My_GPIO_Init(GPIO_TypeDef *GPIOx, const My_GPIO_InitTypeDef *InitStr)
{
  uint32_t pins = (uint32_t)InitStr->PinMask;
  uint32_t new_mode = InitStr->Mode & 3U;
  uint32_t new_type = InitStr->OType & 1U;

  for (int pin = 0; pin < 16; pin++)
  {
    if (pins & (1U << pin))
    {
      uint32_t bit = 1U << pin;
      uint32_t shift = (uint32_t)pin * 2U;
      uint32_t field_mask = 3U << shift;
      uint32_t old_mode = (GPIOx->MODER >> shift) & 3U;
      uint32_t pull = (InitStr->Pull & 3U) << shift;
      uint32_t speed = (InitStr->Speed & 3U) << shift;

      if (old_mode == 1U && new_mode == 1U)
      {
        /* Не отключаем уже работающий GPIO-выход. */
        if (InitStr->InitialState == 0U)
        {
          /* Ноль активно удерживается и push-pull, и open-drain. */
          GPIOx->BSRR = bit << 16U;
        }
        else if (new_type == 0U)
        {
          /* Включаем push-pull до установки конечной единицы. */
          GPIOx->OTYPER = GPIOx->OTYPER & ~bit;
        }

        /* Настраиваем подтяжку до возможного отпускания open-drain. */
        GPIOx->PUPDR = (GPIOx->PUPDR & ~field_mask) | pull;
        GPIOx->OSPEEDR = (GPIOx->OSPEEDR & ~field_mask) | speed;
        GPIOx->OTYPER = (GPIOx->OTYPER & ~bit) | (new_type << pin);

        if (InitStr->InitialState != 0U)
        {
          GPIOx->BSRR = bit;
        }
        continue;
      }

      if (new_mode == 0U)
      {
        /* Подтяжка готова до отключения старого выходного каскада. */
        GPIOx->PUPDR = (GPIOx->PUPDR & ~field_mask) | pull;
        GPIOx->MODER = GPIOx->MODER & ~field_mask;
      }
      else if (new_mode == 3U)
      {
        /* Аналоговый режим отключает выходной каскад и подтяжки. */
        GPIOx->MODER = (GPIOx->MODER & ~field_mask) | (3U << shift);
      }
      else if (old_mode == 2U || new_mode == 2U)
      {
        /* Отключаем AF перед изменением мультиплексора и типа выхода. */
        GPIOx->MODER = GPIOx->MODER & ~field_mask;
      }

      GPIOx->OTYPER = (GPIOx->OTYPER & ~bit) | (new_type << pin);
      GPIOx->OSPEEDR = (GPIOx->OSPEEDR & ~field_mask) | speed;

      if (new_mode == 1U)
      {
        /* Предзагрузка ODR, пока GPIO-выход ещё отключён. */
        GPIOx->BSRR = InitStr->InitialState != 0U ? bit : bit << 16U;

        if (new_type == 1U && InitStr->InitialState != 0U)
        {
          /* Отпущенному open-drain нужна конечная подтяжка сразу. */
          GPIOx->PUPDR = (GPIOx->PUPDR & ~field_mask) | pull;
        }

        GPIOx->MODER = (GPIOx->MODER & ~field_mask) | (1U << shift);

        /* Для PP и OD с нулём применяем подтяжку после включения выхода. */
        GPIOx->PUPDR = (GPIOx->PUPDR & ~field_mask) | pull;
      }
      else if (new_mode == 2U)
      {
        uint32_t af_index = (uint32_t)pin / 8U;
        uint32_t af_shift = ((uint32_t)pin % 8U) * 4U;

        GPIOx->AFR[af_index] =
            (GPIOx->AFR[af_index] & ~(0xFU << af_shift)) |
            ((InitStr->Alternate & 0xFU) << af_shift);
        GPIOx->PUPDR = (GPIOx->PUPDR & ~field_mask) | pull;

        /* Периферия должна быть подготовлена вызывающим кодом. */
        GPIOx->MODER = (GPIOx->MODER & ~field_mask) | (2U << shift);
      }
      else if (new_mode == 3U)
      {
        GPIOx->PUPDR = (GPIOx->PUPDR & ~field_mask) | pull;
      }
    }
  }
}

void My_GPIO_GetConfig(GPIO_TypeDef *GPIOx, uint16_t pin,
                       My_GPIO_InitTypeDef *InitStr)
{
  InitStr->PinMask = (uint16_t)(1U << pin);

  InitStr->Mode = (GPIOx->MODER >> (pin * 2U)) & 3U;

  InitStr->OType = (GPIOx->OTYPER >> pin) & 1U;

  InitStr->Speed = (GPIOx->OSPEEDR >> (pin * 2U)) & 3U;

  InitStr->Pull = (GPIOx->PUPDR >> (pin * 2U)) & 3U;

  InitStr->Alternate = (GPIOx->AFR[pin / 8U] >> ((pin % 8U) * 4U)) & 0xFU;

  InitStr->InitialState = (uint8_t)((GPIOx->ODR >> pin) & 1U);
}

uint16_t My_GPIO_ReadPort(GPIO_TypeDef *GPIOx)
{
  return (uint16_t)(GPIOx->IDR & 0xFFFFU);
}

uint16_t My_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t PinMask)
{
  return ((uint16_t)(GPIOx->IDR & PinMask) != 0U) ? 1U : 0U;
}

void My_GPIO_WritePins(GPIO_TypeDef *GPIOx, uint16_t PinMask,
                       uint16_t StateMask)
{
  uint32_t set_bits = (uint32_t)(StateMask & PinMask);

  uint32_t reset_bits = ((uint32_t)(PinMask & (uint16_t)(~StateMask))) << 16U;

  GPIOx->BSRR = set_bits | reset_bits;
}

// void My_GPIO_WritePins(GPIO_TypeDef *GPIOx, uint16_t PinMask,
//                        uint16_t StateMask)
// {
//   GPIOx->ODR = (GPIOx->ODR & ~PinMask) | (StateMask & PinMask);
// }
