#ifndef FREERTOSQUEUE_UART_DMA_H
#define FREERTOSQUEUE_UART_DMA_H

#include "main.h"
#include "usart.h"
#include "cmsis_os.h"

#define UART_RX_BUF_SIZE 128

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
/* 互斥锁句柄，在 freertos.c 中创建 */
extern osMutexId UARTMutexHandle;

/* Echo任务句柄，用于从中断发送Task Notification */
extern osThreadId EchoTaskHandle;

/* DMA接收缓冲区 */
extern uint8_t uart_rx_buf[UART_RX_BUF_SIZE];

void UART_SendWithMutex(const char *msg);
void UART_StartReceive(void);


#endif //FREERTOSQUEUE_UART_DMA_H