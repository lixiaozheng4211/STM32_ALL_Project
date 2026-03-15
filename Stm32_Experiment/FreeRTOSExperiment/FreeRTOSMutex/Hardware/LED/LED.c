#include "LED.h"

uint16_t LED1_Count = 0, LED2_Count = 0;

void LED_Control(GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin, uint8_t cmd) {
  switch (cmd) {
    case LED_ON:
      HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_SET);
      break;
    case LED_OFF:
      HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_RESET);
      break;
    case LED_TOGGLE:
      HAL_GPIO_TogglePin(GPIO_Port, GPIO_Pin);
      break;
    default:
      break;
  }
  if (GPIO_Pin == GPIO_PIN_0) {
    __disable_irq();
    LED1_Count++;
    __enable_irq();
  } else {
    __disable_irq();
    LED2_Count++;
    __enable_irq();
  }
}
