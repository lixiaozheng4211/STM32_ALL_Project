#include "Task.h"

extern UART_HandleTypeDef huart1;
extern uint8_t ucButtonFlag;
extern uint8_t ucAdcFlag;
extern uint8_t ucUartFlag;

// 只是测试LED
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
  printf("ADC time : %d \r\n", HAL_GetTick());
}

void vTask(void) {
  // 前后台程序分离架构

  if (ucButtonFlag == 1) {
    ucButtonFlag = 0;
    vButtonTask();
  }
  if (ucAdcFlag == 1) {
    ucAdcFlag = 0;
    vAdcTask();
  }
  if (ucUartFlag == 1) {
    ucUartFlag = 0;
    vUartTask();
  }
  HAL_Delay(500);
}
