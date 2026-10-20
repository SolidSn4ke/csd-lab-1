#include "my_gpio.h"

void My_GPIO_Init(GPIO_TypeDef *GPIOx, const My_GPIO_InitTypeDef *InitStr) {
  uint32_t pins = (uint32_t)InitStr->PinMask;

  if ((InitStr->Mode & 3U) == 1U) {
    if (InitStr->InitialState != 0U) {
      GPIOx->BSRR = pins;
    } else {
      GPIOx->BSRR = pins << 16U;
    }
  }

  for (int pin = 0; pin < 16; pin++) {
    if (pins & (1U << pin)) {

      GPIOx->OTYPER =
          (GPIOx->OTYPER & ~(1U << pin)) | ((InitStr->OType & 1U) << pin);

      GPIOx->OSPEEDR = (GPIOx->OSPEEDR & ~(3U << (pin * 2))) |
                       ((InitStr->Speed & 3U) << (pin * 2));

      GPIOx->PUPDR = (GPIOx->PUPDR & ~(3U << (pin * 2))) |
                     ((InitStr->Pull & 3U) << (pin * 2));

      if ((InitStr->Mode & 3U) == 2U) {
        if (pin < 8) {
          GPIOx->AFR[0] = (GPIOx->AFR[0] & ~(0xFU << (pin * 4))) |
                          ((InitStr->Alternate & 0xFU) << (pin * 4));
        } else {
          GPIOx->AFR[1] = (GPIOx->AFR[1] & ~(0xFU << ((pin - 8) * 4))) |
                          ((InitStr->Alternate & 0xFU) << ((pin - 8) * 4));
        }
      }
    }
  }

  for (int pin = 0; pin < 16; pin++) {
    if (pins & (1U << pin)) {
      GPIOx->MODER = (GPIOx->MODER & ~(3U << (pin * 2))) |
                     ((InitStr->Mode & 3U) << (pin * 2));
    }
  }
}

void My_GPIO_GetConfig(GPIO_TypeDef *GPIOx, uint16_t pin,
                       My_GPIO_InitTypeDef *InitStr) {
  InitStr->PinMask = (uint16_t)(1U << pin);

  InitStr->Mode = (GPIOx->MODER >> (pin * 2U)) & 3U;

  InitStr->OType = (GPIOx->OTYPER >> pin) & 1U;

  InitStr->Speed = (GPIOx->OSPEEDR >> (pin * 2U)) & 3U;

  InitStr->Pull = (GPIOx->PUPDR >> (pin * 2U)) & 3U;

  InitStr->Alternate = (GPIOx->AFR[pin / 8U] >> ((pin % 8U) * 4U)) & 0xFU;

  InitStr->InitialState = (uint8_t)((GPIOx->ODR >> pin) & 1U);
}

uint16_t My_GPIO_ReadPort(GPIO_TypeDef *GPIOx) {
  return (uint16_t)(GPIOx->IDR & 0xFFFFU);
}

uint16_t My_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t PinMask) {
  return ((uint16_t)(GPIOx->IDR & PinMask) != 0U) ? 1U : 0U;
}

void My_GPIO_WritePins(GPIO_TypeDef *GPIOx, uint16_t PinMask,
                       uint16_t StateMask) {
  uint32_t set_bits = (uint32_t)(StateMask & PinMask);

  uint32_t reset_bits = ((uint32_t)(PinMask & (uint16_t)(~StateMask))) << 16U;

  GPIOx->BSRR = set_bits | reset_bits;
}

/*
void My_GPIO_WritePins(GPIO_TypeDef *GPIOx, uint16_t PinMask,
                       uint16_t StateMask) {
  GPIOx->ODR = (GPIOx->ODR & ~PinMask) | (StateMask & PinMask);
}
*/
