#ifndef UART_DMA_PRINTF_H
#define UART_DMA_PRINTF_H
#include "main.h"
#include <stdint.h>


#define UART_RX_BUF_SIZE 128
extern  uint8_t uart_rx_buf[UART_RX_BUF_SIZE];
#define UART_TX_BUF_SIZE 512

void UART_Printf_Init(UART_HandleTypeDef *huart);

void UART_Printf_Push(const uint8_t *data, uint16_t len);

void UART_Printf_TaskKick(void);



/* 计算环形缓冲区已有数据长度 */
static uint16_t rb_data_len(void);

/* 计算环形缓冲区剩余空间 */
static uint16_t rb_free_len(void);

/* 从环形缓冲区取一段连续数据到 dma_chunk_buf，并启动 DMA */
static void uart_tx_start_dma_if_needed(void);

void UART_Printf_Init(UART_HandleTypeDef *huart);

/* 往环形缓冲区塞数据 */
void UART_Printf_Push(const uint8_t *data, uint16_t len);

/* 尝试启动一次 DMA */
void UART_Printf_TaskKick(void);

/* 这个函数要在 HAL_UART_TxCpltCallback 里调用 */
void UART_Printf_OnTxCplt(UART_HandleTypeDef *huart);

#endif
