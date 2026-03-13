#include "uart_dma_printf.h"
#include <string.h>
static UART_HandleTypeDef *g_uart = NULL;
uint8_t uart_rx_buf[UART_RX_BUF_SIZE];
/* 环形缓冲区 */
static uint8_t tx_buf[UART_TX_BUF_SIZE];
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;

/* DMA 当前发送信息 */
static uint8_t dma_chunk_buf[UART_TX_BUF_SIZE];
static volatile uint16_t dma_chunk_len = 0;
static volatile uint8_t uart_tx_busy = 0;

/* 计算环形缓冲区已有数据长度 */
static uint16_t rb_data_len(void) {
  if (tx_head >= tx_tail) {
    return tx_head - tx_tail;
  } else {
    return UART_TX_BUF_SIZE - tx_tail + tx_head;
  }
}

/* 计算环形缓冲区剩余空间 */
static uint16_t rb_free_len(void) {
  return UART_TX_BUF_SIZE - rb_data_len() - 1;
}

/* 从环形缓冲区取一段连续数据到 dma_chunk_buf，并启动 DMA */
static void uart_tx_start_dma_if_needed(void) {
  if (g_uart == NULL) {
    return;
  }

  if (uart_tx_busy) {
    return;
  }

  if (tx_head == tx_tail) {
    return;
  }

  /* 取一段连续区间，避免 DMA 跨环 */
  uint16_t len;
  if (tx_head > tx_tail) {
    len = tx_head - tx_tail;
  } else {
    len = UART_TX_BUF_SIZE - tx_tail;
  }

  if (len == 0) {
    return;
  }

  memcpy(dma_chunk_buf, &tx_buf[tx_tail], len);
  dma_chunk_len = len;
  uart_tx_busy = 1;

  if (HAL_UART_Transmit_DMA(g_uart, dma_chunk_buf, dma_chunk_len) != HAL_OK) {
    uart_tx_busy = 0;
  }
}

void UART_Printf_Init(UART_HandleTypeDef *huart) {
  g_uart = huart;
  tx_head = 0;
  tx_tail = 0;
  dma_chunk_len = 0;
  uart_tx_busy = 0;
}

/* 往环形缓冲区塞数据 */
void UART_Printf_Push(const uint8_t *data, uint16_t len) {
  if (g_uart == NULL || data == NULL || len == 0) {
    return;
  }

  for (uint16_t i = 0; i < len; i++) {
    uint16_t next = (uint16_t) ((tx_head + 1) % UART_TX_BUF_SIZE);

    /* 缓冲区满了：这里先选择丢数据
       你也可以改成阻塞等待、覆盖旧数据，或者返回错误 */
    if (next == tx_tail) {
      break;
    }

    tx_buf[tx_head] = data[i];
    tx_head = next;
  }

  UART_Printf_TaskKick();
}

/* 尝试启动一次 DMA */
void UART_Printf_TaskKick(void) {
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  uart_tx_start_dma_if_needed();
  if (!primask) {
    __enable_irq();
  }
}

/* 这个函数要在 HAL_UART_TxCpltCallback 里调用 */
void UART_Printf_OnTxCplt(UART_HandleTypeDef *huart) {
  if (huart != g_uart) {
    return;
  }

  /* 当前这一段发完了，tail 前移 */
  tx_tail = (uint16_t) ((tx_tail + dma_chunk_len) % UART_TX_BUF_SIZE);
  dma_chunk_len = 0;
  uart_tx_busy = 0;

  uart_tx_start_dma_if_needed();
}
