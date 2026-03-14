#include "UART_DMA.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ---- 临界区 ---- */
static uint32_t enter_critical(void) {
    uint32_t p = __get_PRIMASK();
    __disable_irq();
    return p;
}
static void exit_critical(uint32_t p) {
    if (!p) __enable_irq();
}

/* ---- RX 环形队列写入 ---- */
static void rx_push(UART_IO_t *inst, uint8_t ch) {
    uint16_t next = (uint16_t)((inst->rx_head + 1) % UART_RX_RING_SIZE);
    if (next == inst->rx_tail) return;
    inst->rx_ring[inst->rx_head] = ch;
    inst->rx_head = next;
}

/* ---- TX 环形队列写入 ---- */
static void tx_push(UART_IO_t *inst, const uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        uint16_t next = (uint16_t)((inst->tx_head + 1) % UART_TX_RING_SIZE);
        if (next == inst->tx_tail) break;
        inst->tx_ring[inst->tx_head] = data[i];
        inst->tx_head = next;
    }
}

/* ---- 从 TX 环形队列取连续块到 tx_dma_buf ---- */
static uint16_t tx_prepare(UART_IO_t *inst) {
    if (inst->tx_head == inst->tx_tail) return 0;
    uint16_t len;
    if (inst->tx_head > inst->tx_tail)
        len = (uint16_t)(inst->tx_head - inst->tx_tail);
    else
        len = (uint16_t)(UART_TX_RING_SIZE - inst->tx_tail);
    memcpy(inst->tx_dma_buf, &inst->tx_ring[inst->tx_tail], len);
    return len;
}

/* ---- 启动 TX DMA ---- */
static void tx_kick(UART_IO_t *inst) {
    if (inst->huart == NULL || inst->tx_busy) return;
    inst->tx_dma_len = tx_prepare(inst);
    if (inst->tx_dma_len == 0) return;
    inst->tx_busy = 1;
    if (HAL_UART_Transmit_DMA(inst->huart, inst->tx_dma_buf, inst->tx_dma_len) != HAL_OK) {
        inst->tx_busy = 0;
        inst->tx_dma_len = 0;
    }
}

/* ---- 公开接口 ---- */

void UART_IO_Init(UART_IO_t *inst, UART_HandleTypeDef *huart) {
    memset(inst, 0, sizeof(UART_IO_t));
    inst->huart = huart;
}

void UART_IO_StartReceive(UART_IO_t *inst) {
    if (inst->huart == NULL) return;
    HAL_UART_Receive_DMA(inst->huart, inst->rx_dma_buf, UART_RX_DMA_SIZE);
    __HAL_UART_ENABLE_IT(inst->huart, UART_IT_IDLE);
}

void UART_IO_Send(UART_IO_t *inst, const uint8_t *data, uint16_t len) {
    if (inst->huart == NULL || data == NULL || len == 0) return;
    uint32_t p = enter_critical();
    tx_push(inst, data, len);
    tx_kick(inst);
    exit_critical(p);
}

void UART_IO_Printf(UART_IO_t *inst, const char *fmt, ...) {
    char tmp[UART_PRINTF_TMP_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    if (len <= 0) return;
    if (len > (int)sizeof(tmp)) len = (int)sizeof(tmp);
    UART_IO_Send(inst, (const uint8_t *)tmp, (uint16_t)len);
}

void UART_IO_TxCpltCallback(UART_IO_t *inst, UART_HandleTypeDef *huart) {
    if (inst->huart == NULL || huart != inst->huart) return;
    uint32_t p = enter_critical();
    inst->tx_tail = (uint16_t)((inst->tx_tail + inst->tx_dma_len) % UART_TX_RING_SIZE);
    inst->tx_dma_len = 0;
    inst->tx_busy = 0;
    tx_kick(inst);
    exit_critical(p);
}

void UART_IO_IdleHandler(UART_IO_t *inst, UART_HandleTypeDef *huart) {
    if (inst->huart == NULL || huart != inst->huart) return;
    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET) {
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        HAL_UART_DMAStop(huart);
        uint16_t recv_len = (uint16_t)(UART_RX_DMA_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx));
        for (uint16_t i = 0; i < recv_len; i++) {
            rx_push(inst, inst->rx_dma_buf[i]);
        }
        HAL_UART_Receive_DMA(huart, inst->rx_dma_buf, UART_RX_DMA_SIZE);
    }
}

uint8_t UART_IO_ReadByte(UART_IO_t *inst, uint8_t *ch) {
    if (ch == NULL) return 0;
    uint32_t p = enter_critical();
    if (inst->rx_head == inst->rx_tail) {
        exit_critical(p);
        return 0;
    }
    *ch = inst->rx_ring[inst->rx_tail];
    inst->rx_tail = (uint16_t)((inst->rx_tail + 1) % UART_RX_RING_SIZE);
    exit_critical(p);
    return 1;
}
