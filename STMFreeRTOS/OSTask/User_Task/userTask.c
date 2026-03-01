#include "userTask.h"
#include "cmsis_os2.h"
#include "freertos_os2.h"
#include "task.h"


void vButtonTask(void) {
  if (HAL_GPIO_ReadPin(KEY1_GPIO_Port , KEY1_Pin) == GPIO_PIN_RESET) { // 表示没按下
    HAL_GPIO_WritePin(LED_BOARD_GPIO_Port , LED_BOARD_Pin, GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(LED_BOARD_GPIO_Port , LED_BOARD_Pin, GPIO_PIN_SET);
  }
}

void vAdcTask(void) {
  printf("ADC Task %ld \r\n",HAL_GetTick());
  osDelay(300);
}

void vUartTask(void) {
  printf("uart Task %ld \r\n",HAL_GetTick());
  osDelay(1000);
}

void vUartTask_A(void) {
  printf("A\r\n");
  portYIELD(); // 告诉调度器可以开始切换任务
  osDelay(1000);
}

void vUartTask_B(void) {
  printf("B\r\n");
  osDelay(1000);
}

void vUartTask_C(void) {
  printf("C\r\n");
  osDelay(1000);
}