#ifndef UART_DMA_H
#define UART_DMA_H

#include "main.h"
#include <stddef.h>
#include <stdint.h>

/*=========================
 * 可调参数
 *=========================*/
#define UART_TX_RING_SIZE    512
#define UART_RX_RING_SIZE    512
#define UART_RX_DMA_SIZE     128
#define UART_PRINTF_TMP_SIZE 256

/*=========================
 * 串口实例结构体
 *=========================*/
typedef struct {
    UART_HandleTypeDef *huart;

    /* TX 环形队列 */
    uint8_t           tx_ring[UART_TX_RING_SIZE];
    volatile uint16_t tx_head;
    volatile uint16_t tx_tail;
    uint8_t           tx_dma_buf[UART_TX_RING_SIZE];
    volatile uint16_t tx_dma_len;
    volatile uint8_t  tx_busy;

    /* RX DMA 缓冲区 + 环形队列 */
    uint8_t           rx_dma_buf[UART_RX_DMA_SIZE];
    uint8_t           rx_ring[UART_RX_RING_SIZE];
    volatile uint16_t rx_head;
    volatile uint16_t rx_tail;
} UART_IO_t;

void    UART_IO_Init(UART_IO_t *inst, UART_HandleTypeDef *huart);
void    UART_IO_StartReceive(UART_IO_t *inst);
void    UART_IO_Send(UART_IO_t *inst, const uint8_t *data, uint16_t len);
void    UART_IO_Printf(UART_IO_t *inst, const char *fmt, ...);
uint8_t UART_IO_ReadByte(UART_IO_t *inst, uint8_t *ch);
void    UART_IO_IdleHandler(UART_IO_t *inst, UART_HandleTypeDef *huart);
void    UART_IO_TxCpltCallback(UART_IO_t *inst, UART_HandleTypeDef *huart);

#endif /* UART_DMA_H */
