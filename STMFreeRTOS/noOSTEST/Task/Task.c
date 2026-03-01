#include "Task.h"


void vButtonTask(void) {
  if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, GPIO_PIN_1) == 0) {
    HAL_Delay(20);
    if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, GPIO_PIN_1) == 0) {
      HAL_GPIO_WritePin(LED_BOARD_GPIO_Port, GPIO_PIN_1, GPIO_PIN_RESET);
    }
  } else {
    HAL_GPIO_WritePin(LED_BOARD_GPIO_Port, GPIO_PIN_1, GPIO_PIN_SET);
  }
}


void vUartTask(void) {
  printf("uart time : %d \r\n", HAL_GetTick());
}


void vAdcTask(void) {
  HAL_Delay(200);
  printf("ADC time : %d \r\n", HAL_GetTick());
}

void vTask(void) {
  vButtonTask();
  vAdcTask();
  vUartTask();

  HAL_Delay(500);
}
