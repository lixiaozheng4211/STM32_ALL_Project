#include "LED.h"

void LED_Control(GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin, uint8_t cmd) {
  if (cmd == LED_ON) {
    HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_SET);
  } else if (cmd == LED_OFF) {
    HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_RESET);
  } else {
    HAL_GPIO_TogglePin(GPIO_Port, GPIO_Pin);
  }
}
