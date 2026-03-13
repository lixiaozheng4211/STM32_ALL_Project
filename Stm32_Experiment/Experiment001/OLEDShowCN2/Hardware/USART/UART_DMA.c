#include "uart_dma.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/*=========================
 * 可调参数
 *=========================*/
#define UART_TX_RING_SIZE   512
#define UART_RX_RING_SIZE   512
#define UART_RX_DMA_SIZE    128
#define UART_PRINTF_TMP_SIZE 256

/*=========================
 * 静态变量
 *=========================*/
static UART_HandleTypeDef *g_uart = NULL;

/*----------- TX: 环形队列 -----------*/
static uint8_t tx_ring[UART_TX_RING_SIZE];
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;

/* 当前 DMA 正在发送的连续块，先拷到这里 */
static uint8_t tx_dma_buf[UART_TX_RING_SIZE];
static volatile uint16_t tx_dma_len = 0;
static volatile uint8_t tx_busy = 0;

/*----------- RX: DMA 缓冲区 + 环形队列 -----------*/
static uint8_t rx_dma_buf[UART_RX_DMA_SIZE];

static uint8_t rx_ring[UART_RX_RING_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

/*=========================
 * 小工具函数：进入/退出临界区
 *=========================*/
static uint32_t uart_io_enter_critical(void) {
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  return primask;
}

static void uart_io_exit_critical(uint32_t primask) {
  if (!primask) {
    __enable_irq();
  }
}

/*=========================
 * RX 环形队列：写 1 字节
 * 满了就丢弃新数据
 *=========================*/
static void rx_ring_push(uint8_t ch) {
  uint16_t next = (uint16_t)((rx_head + 1) % UART_RX_RING_SIZE);
  if (next == rx_tail) {
    /* 满了，丢数据 */
    return;
  }
  rx_ring[rx_head] = ch;
  rx_head = next;
}

/*=========================
 * TX 环形队列：写入多字节
 * 满了就停止写入（丢后续数据）
 *=========================*/
static void tx_ring_push(const uint8_t *data, uint16_t len) {
  uint16_t i;
  for (i = 0; i < len; i++) {
    uint16_t next = (uint16_t)((tx_head + 1) % UART_TX_RING_SIZE);
    if (next == tx_tail) {
      /* TX 环形缓冲区满了，后续数据丢弃 */
      break;
    }
    tx_ring[tx_head] = data[i];
    tx_head = next;
  }
}

/*=========================
 * 从 TX 环形队列里取出一段“连续内存”
 * 拷到 tx_dma_buf 里，准备发 DMA
 *
 * 为什么要拷到 tx_dma_buf？
 * 因为环形队列的数据可能跨尾部，DMA 最好发连续内存
 *=========================*/
static uint16_t tx_prepare_dma_chunk(void) {
  uint16_t len = 0;

  if (tx_head == tx_tail) {
    return 0;
  }

  if (tx_head > tx_tail) {
    /* 这一段本来就是连续的 */
    len = (uint16_t)(tx_head - tx_tail);
  } else {
    /* 队列跨尾了，先发 tail 到末尾这一段 */
    len = (uint16_t)(UART_TX_RING_SIZE - tx_tail);
  }

  memcpy(tx_dma_buf, &tx_ring[tx_tail], len);
  return len;
}

/*=========================
 * 尝试启动一次 TX DMA
 *
 * 条件：
 * 1. 串口句柄有效
 * 2. 当前没有忙
 * 3. 队列里有数据
 *=========================*/
static void tx_kick_dma(void) {
  if (g_uart == NULL) {
    return;
  }

  if (tx_busy) {
    return;
  }

  tx_dma_len = tx_prepare_dma_chunk();
  if (tx_dma_len == 0) {
    return;
  }

  tx_busy = 1;

  if (HAL_UART_Transmit_DMA(g_uart, tx_dma_buf, tx_dma_len) != HAL_OK) {
    /* 启动失败，恢复 busy */
    tx_busy = 0;
    tx_dma_len = 0;
  }
}

/*=========================
 * 初始化
 *=========================*/
void UART_IO_Init(UART_HandleTypeDef *huart) {
  g_uart = huart;

  tx_head = 0;
  tx_tail = 0;
  tx_dma_len = 0;
  tx_busy = 0;

  rx_head = 0;
  rx_tail = 0;
}

/*=========================
 * 启动 RX DMA + IDLE
 *
 * 注意：
 * 1. 你必须已经在 CubeMX 里开好 UART RX DMA
 * 2. 你必须打开 USART 全局中断
 * 3. 你必须在 IRQHandler 里调用 UART_IO_IdleHandler()
 *=========================*/
void UART_IO_StartReceive(void) {
  if (g_uart == NULL) {
    return;
  }

  HAL_UART_Receive_DMA(g_uart, rx_dma_buf, UART_RX_DMA_SIZE);

  /* 开启 IDLE 中断 */
  __HAL_UART_ENABLE_IT(g_uart, UART_IT_IDLE);
}

/*=========================
 * 发送接口：原始数据
 * 流程：
 * 1. 塞进 TX 环形队列
 * 2. 尝试启动 DMA
 *=========================*/
void UART_IO_Send(const uint8_t *data, uint16_t len) {
  uint32_t primask;

  if (g_uart == NULL || data == NULL || len == 0) {
    return;
  }

  primask = uart_io_enter_critical();

  tx_ring_push(data, len);
  tx_kick_dma();

  uart_io_exit_critical(primask);
}

/*=========================
 * printf 风格打印
 *=========================*/
void uart_printf(const char *fmt, ...) {
  char tmp[UART_PRINTF_TMP_SIZE];
  va_list ap;
  int len;

  va_start(ap, fmt);
  len = vsnprintf(tmp, sizeof(tmp), fmt, ap);
  va_end(ap);

  if (len <= 0) {
    return;
  }

  if (len > (int) sizeof(tmp)) {
    len = sizeof(tmp);
  }

  UART_IO_Send((const uint8_t *) tmp, (uint16_t) len);
}

/*=========================
 * printf 重定向
 * 如果你启用这个，就可以直接 printf()
 *=========================*/
int _write(int file, char *ptr, int len) {
  (void) file;

  if (ptr == NULL || len <= 0) {
    return 0;
  }

  UART_IO_Send((const uint8_t *) ptr, (uint16_t) len);
  return len;
}

/*=========================
 * DMA 发送完成回调
 *
 * 你需要在 HAL_UART_TxCpltCallback() 里调用它
 *
 * 做的事：
 * 1. 把 tx_tail 前移，表示刚刚这段已经发掉了
 * 2. busy 清零
 * 3. 如果队列里还有数据，再启动下一次 DMA
 *=========================*/
void UART_IO_TxCpltCallback(UART_HandleTypeDef *huart) {
  uint32_t primask;

  if (g_uart == NULL || huart != g_uart) {
    return;
  }

  primask = uart_io_enter_critical();

  tx_tail = (uint16_t)((tx_tail + tx_dma_len) % UART_TX_RING_SIZE);
  tx_dma_len = 0;
  tx_busy = 0;

  tx_kick_dma();

  uart_io_exit_critical(primask);
}

/*=========================
 * UART IDLE 中断处理
 *
 * 你要在 USARTx_IRQHandler() 里调用它
 *
 * 做的事：
 * 1. 检查是否触发 IDLE
 * 2. 清 IDLE 标志
 * 3. 停 DMA
 * 4. 计算本次收到的字节数
 * 5. 拷贝到 RX 环形队列
 * 6. 重启 DMA
 *=========================*/
void UART_IO_IdleHandler(UART_HandleTypeDef *huart) {
  uint16_t recv_len;
  uint16_t i;

  if (g_uart == NULL || huart != g_uart) {
    return;
  }

  if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET) {
    /* 清 IDLE 标志 */
    __HAL_UART_CLEAR_IDLEFLAG(huart);

    /* 先停 DMA，冻结当前计数器 */
    HAL_UART_DMAStop(huart);

    /* 计算实际收到多少字节
     * DMA 剩余计数 = __HAL_DMA_GET_COUNTER(huart->hdmarx)
     * 已收长度 = 总长度 - 剩余长度
     */
    recv_len = (uint16_t)(UART_RX_DMA_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx));

    /* 把这次收到的数据搬到 RX 环形队列
     * 为什么还要再拷一份到 RX 环形队列？
     * 因为 rx_dma_buf 是“本次 DMA 临时接收区”
     * 而 rx_ring 才是“主循环慢慢取数据的队列”
     */
    for (i = 0; i < recv_len; i++) {
      rx_ring_push(rx_dma_buf[i]);
    }

    /* 重新开启 DMA，准备下一帧 */
    HAL_UART_Receive_DMA(huart, rx_dma_buf, UART_RX_DMA_SIZE);
  }
}

/*=========================
 * 主循环里每次调一下，读取 1 字节
 *=========================*/
uint8_t UART_IO_ReadByte(uint8_t *ch) {
  uint32_t primask;

  if (ch == NULL) {
    return 0;
  }

  primask = uart_io_enter_critical();

  if (rx_head == rx_tail) {
    uart_io_exit_critical(primask);
    return 0;
  }

  *ch = rx_ring[rx_tail];
  rx_tail = (uint16_t)((rx_tail + 1) % UART_RX_RING_SIZE);

  uart_io_exit_critical(primask);
  return 1;
}
