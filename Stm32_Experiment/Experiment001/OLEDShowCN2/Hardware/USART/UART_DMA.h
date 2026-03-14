#ifndef UART_DMA_H
#define UART_DMA_H

#include "main.h"
#include <stddef.h>
#include <stdint.h>


/* 初始化模块，传入你要用的串口句柄 */
void UART_IO_Init(UART_HandleTypeDef *huart);

/* 启动 RX DMA + IDLE */
void UART_IO_StartReceive(void);

/* 发送原始数据（走 TX 环形队列 + DMA） */
void UART_IO_Send(const uint8_t *data, uint16_t len);

/* printf 风格打印 */
void uart_printf(const char *fmt, ...);

/* 从 RX 环形队列中读取 1 字节
 * 返回 1 表示读到
 * 返回 0 表示没有数据
 */
uint8_t UART_IO_ReadByte(uint8_t *ch);

/* 这两个函数要在回调/中断里调用 */
void UART_IO_TxCpltCallback(UART_HandleTypeDef *huart);
void UART_IO_IdleHandler(UART_HandleTypeDef *huart);
/* 如果你想把 printf 重定向到这里，可打开这个接口 */
int _write(int file, char *ptr, int len);

#endif