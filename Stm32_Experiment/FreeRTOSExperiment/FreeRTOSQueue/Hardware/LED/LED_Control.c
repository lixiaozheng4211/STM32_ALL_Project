#include "LED_Control.h"

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
}
