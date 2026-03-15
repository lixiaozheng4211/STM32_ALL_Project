#include "UART_DMA.h"
#include "usart.h"
#include <string.h>

uint8_t uart_rx_buf[UART_RX_BUF_SIZE];

/**
 * @brief 带互斥锁保护的串口DMA发送
 */
void UART_SendWithMutex(const char *msg) {
  uint16_t len = (uint16_t) strlen(msg);

  if (osMutexWait(UARTMutexHandle, 1000) == osOK) {
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *) msg, len);

    while (HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX ||
           HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX_RX) {
      osDelay(1);
           }

    osMutexRelease(UARTMutexHandle);
  }
}

/**
 * @brief 开启一次不定长DMA接收（空闲中断触发）
 * 每次接收完成后需要重新调用此函数
 */
void UART_StartReceive(void) {
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_buf, UART_RX_BUF_SIZE);
  /* 关闭DMA半传输中断，避免数据只收一半就触发回调 */
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
}

/**
 * @brief UART空闲中断/DMA完成回调（在中断上下文中执行）
 *
 * HAL_UARTEx_ReceiveToIdle_DMA 触发后会调用此函数。
 * 这里不做任何处理，只通过 Task Notification 把接收长度
 * 传给 Echo 任务，让任务在线程上下文里完成发送。
 *
 * @param huart  UART句柄
 * @param Size   实际接收到的字节数
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
  if (huart->Instance == USART1) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* 把接收长度作为通知值发给Echo任务 */
    xTaskNotifyFromISR(
      EchoTaskHandle,
      Size,
      eSetValueWithOverwrite,
      &xHigherPriorityTaskWoken
    );

    /* 如果唤醒了更高优先级的任务，请求上下文切换 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}
