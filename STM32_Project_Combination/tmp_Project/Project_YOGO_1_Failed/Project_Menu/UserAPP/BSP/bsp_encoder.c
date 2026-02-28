#include "bsp_encoder.h"

/* ---- 配置 ---- */
#define COUNTS_PER_DETENT   4      /* 编码器每个棘格的计数数 */
#define LONG_PRESS_TICKS    200    /* 长按阈值 = 200 * 5ms = 1s */
#define DEBOUNCE_TICKS      4      /* 消抖 = 4 * 5ms = 20ms */

/* ---- 按键状态机 ---- */
typedef enum {
    KEY_IDLE,
    KEY_DEBOUNCE_DOWN,
    KEY_PRESSED_HOLD,
    KEY_WAIT_RELEASE
} Key_State_t;

/* ---- 事件队列 ---- */
#define EVT_QUEUE_SIZE  8

static Encoder_Event_t evt_queue[EVT_QUEUE_SIZE];
static uint8_t q_head = 0;
static uint8_t q_tail = 0;

static TIM_HandleTypeDef *enc_htim;
static int16_t last_cnt = 0;
static int16_t accum = 0;

static Key_State_t key_state = KEY_IDLE;
static uint16_t key_ticks = 0;

/* ---- 内部函数 ---- */
static void evt_push(Encoder_Event_t evt)
{
    uint8_t next = (q_head + 1) % EVT_QUEUE_SIZE;
    if (next != q_tail) {
        evt_queue[q_head] = evt;
        q_head = next;
    }
}

static Encoder_Event_t evt_pop(void)
{
    if (q_head == q_tail)
        return ENC_EVT_NONE;
    Encoder_Event_t evt = evt_queue[q_tail];
    q_tail = (q_tail + 1) % EVT_QUEUE_SIZE;
    return evt;
}

static uint8_t key_read(void)
{
    return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_RESET) ? 1 : 0;
}

/* ---- 公开接口 ---- */
void BSP_Encoder_Init(TIM_HandleTypeDef *htim)
{
    enc_htim = htim;
    HAL_TIM_Encoder_Start(enc_htim, TIM_CHANNEL_ALL);
    __HAL_TIM_SET_COUNTER(enc_htim, 0);
    last_cnt = 0;
    accum = 0;
    q_head = 0;
    q_tail = 0;
    key_state = KEY_IDLE;
}

void BSP_Encoder_Scan(void)
{
    /* ---- 旋转检测 ---- */
    int16_t cnt = (int16_t)__HAL_TIM_GET_COUNTER(enc_htim);
    int16_t diff = cnt - last_cnt;
    last_cnt = cnt;

    accum += diff;
    while (accum >= COUNTS_PER_DETENT) {
        accum -= COUNTS_PER_DETENT;
        evt_push(ENC_EVT_CW);
    }
    while (accum <= -COUNTS_PER_DETENT) {
        accum += COUNTS_PER_DETENT;
        evt_push(ENC_EVT_CCW);
    }

    /* ---- 按键状态机 ---- */
    uint8_t pressed = key_read();

    switch (key_state) {
    case KEY_IDLE:
        if (pressed) {
            key_state = KEY_DEBOUNCE_DOWN;
            key_ticks = 0;
        }
        break;

    case KEY_DEBOUNCE_DOWN:
        key_ticks++;
        if (!pressed) {
            key_state = KEY_IDLE;
        } else if (key_ticks >= DEBOUNCE_TICKS) {
            key_state = KEY_PRESSED_HOLD;
            key_ticks = 0;
        }
        break;

    case KEY_PRESSED_HOLD:
        key_ticks++;
        if (!pressed) {
            evt_push(ENC_EVT_SHORT_PRESS);
            key_state = KEY_IDLE;
        } else if (key_ticks >= LONG_PRESS_TICKS) {
            evt_push(ENC_EVT_LONG_PRESS);
            key_state = KEY_WAIT_RELEASE;
        }
        break;

    case KEY_WAIT_RELEASE:
        if (!pressed) {
            key_state = KEY_IDLE;
        }
        break;
    }
}

Encoder_Event_t BSP_Encoder_GetEvent(void)
{
    return evt_pop();
}
