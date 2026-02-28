#include "bsp_encoder.h"
#include "tim.h" 

#define TIM_USED htim2

// 状态机枚举
typedef enum {
    KEY_IDLE = 0,       // 空闲状态
    KEY_DEBOUNCE,       // 消抖确认中
    KEY_PRESSED,        // 已按下，正在计时区分长短按
    KEY_WAIT_RELEASE    // 长按已触发，等待手松开
} Key_State_t;

// 内部变量定义
static uint16_t    last_count = 0;                   // 记录上一次的定时器计数值
static Key_State_t current_key_state = KEY_IDLE;     // 按键当前状态
static uint16_t    key_hold_ticks = 0;               // 按键按下的时间计数器


#define ENC_PULSE_PER_STEP 4   // 编码器转一格的脉冲数 (常见模块是4，也有2或1的)
#define LONG_PRESS_TICKS   75 // 长按阈值 (假设10ms扫一次，100次=1000ms=1秒)


void BSP_Encoder_Init(void) {
    HAL_TIM_Encoder_Start(&TIM_USED, TIM_CHANNEL_ALL);
    last_count = __HAL_TIM_GET_COUNTER(&TIM_USED);
}


Encoder_Event_t BSP_Encoder_Scan(void) {
    Encoder_Event_t event = ENC_EVENT_NONE;
    uint16_t current_count = __HAL_TIM_GET_COUNTER(&TIM_USED);
    int16_t delta = (int16_t)(current_count - last_count);

    if (delta >= ENC_PULSE_PER_STEP) {
        last_count = current_count;
        return ENC_EVENT_UP;
    }
    else if (delta <= -ENC_PULSE_PER_STEP) {
        last_count = current_count;
        return ENC_EVENT_DOWN;
    }

    uint8_t pin_status = HAL_GPIO_ReadPin(ENCODER_SW_GPIO_Port, ENCODER_SW_Pin);
    switch (current_key_state) {
        case KEY_IDLE:
            if (pin_status == GPIO_PIN_RESET) {
                current_key_state = KEY_DEBOUNCE; // 发现疑似按下，进入消抖
            }
            break;

        case KEY_DEBOUNCE:
            if (pin_status == GPIO_PIN_RESET) {
                current_key_state = KEY_PRESSED;
                key_hold_ticks = 0;               // 开始记录按下的时间
            } else {
                HAL_Delay(1);
                current_key_state = KEY_IDLE;     // 误触
            }
            break;

        case KEY_PRESSED:
            if (pin_status == GPIO_PIN_RESET) {
                key_hold_ticks++; // 手还没松，计数器累加

                if (key_hold_ticks >= LONG_PRESS_TICKS) {
                    event = ENC_EVENT_LONG_PRESS;         // 长按
                    current_key_state = KEY_WAIT_RELEASE; // 转移到等待松开，防止重复触发
                }
            } else {
                // 在长按阈值到来之前就松手了，判定为短按
                event = ENC_EVENT_SHORT_PRESS;            // 短按
                current_key_state = KEY_IDLE;             // 状态机复位
            }
            break;

        case KEY_WAIT_RELEASE:
            if (pin_status == GPIO_PIN_SET) {
                current_key_state = KEY_IDLE;             // 终于松手了，复位
            }
            break;
    }

    return event;
}