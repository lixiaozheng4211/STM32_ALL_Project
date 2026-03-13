#include "KEY_TABLE.h"
#include "stdio.h"

// ==================== 3. 建立状态流转表 ====================
// 这就是整个状态机的"大脑"，所有的规则都在这里
const StateMachineRow_t button_state_table[] = {
    /* 当前状态       发生的事件             目标状态        需要执行的动作 */
    {KEY_NULL, EVENT_PIN_FALLING, KEY_DOWN, Action_KeyDown},
    {KEY_NULL, EVENT_TICK, KEY_NULL, Action_NullTick},
    {KEY_DOWN, EVENT_TICK, KEY_PRESS, Action_None},
    {KEY_PRESS, EVENT_PIN_RISING, KEY_UP, Action_KeyUp},
    {KEY_PRESS, EVENT_TICK, KEY_PRESS, Action_Pressing},
    {KEY_UP, EVENT_TICK, KEY_NULL, Action_None},
};

// Constants for better readability
#define LONG_PRESS_TIME 100
#define BUTTON_COUNT 2

ButtonEvent_t Get_Button_Event(buttonType *btn) {
    uint8_t pinState = HAL_GPIO_ReadPin(btn->GPIO_Port, btn->GPIO_Pin);
    // 只有在空闲时且引脚为低，才算作下降沿事件
    if (btn->keyAction == KEY_NULL && pinState == 0) return EVENT_PIN_FALLING;
    // 只有在按压时且引脚为高，才算作上升沿事件
    if (btn->keyAction == KEY_PRESS && pinState == 1) return EVENT_PIN_RISING;
    return EVENT_TICK;
}

// ==================== 5. 极简的核心引擎 ====================
// 以后不管加多少个状态，这个函数永远都不用改了
void Key_Scan_Engine(buttonType *btn) {
    ButtonEvent_t currentEvent = Get_Button_Event(btn);
    for (int i = 0; i < TABLE_SIZE; i++) {
        // 如果"当前状态"和"发生的事件"在表里匹配上了
        if (button_state_table[i].currentState == btn->keyAction && button_state_table[i].triggerEvent == currentEvent) {
            // 1. 执行伴随动作
            if (button_state_table[i].actionFunc != NULL) {
                button_state_table[i].actionFunc(btn);
            }
            // 2. 状态切换
            btn->keyAction = button_state_table[i].nextState;
            // 匹配成功就可以退出查表了
            break;
        }
    }
}

// 动作：记录按下瞬间
void Action_KeyDown(buttonType *btn) {
    btn->press_flag++;
    btn->release_flag = 0;
    btn->hold_cnt = 0;
}

// 动作：持续按压时的计时
void Action_Pressing(buttonType *btn) {
    btn->hold_cnt++;
}

// 动作：松开瞬间的处理
void Action_KeyUp(buttonType *btn) {
    btn->release_flag = 1;
    btn->high_cnt = 0;
    // 抵消长按造成的无效 press_flag 累加
    // 如果这次按压超过长按时间，则抵消 press_flag
    if (btn->hold_cnt > LONG_PRESS_TIME) {
        btn->press_flag--;
    }
}

// 动作：在 NULL 状态下熬时间并结算 (核心业务逻辑)
void Action_NullTick(buttonType *btn) {
    if (btn->release_flag == 1) {
        btn->high_cnt++;
    }
    // 结算逻辑
    if (btn->hold_cnt > LONG_PRESS_TIME) {
        btn->buttonAction = BUTTON_LONG_PRESS;
        Key_ParaInit(btn);
    }
    // 僵尸按判定：在有效范围和长按之间的无效按压
    else if (btn->hold_cnt > CLICK_MAX_TIME && btn->hold_cnt < LONG_PRESS_TIME) {
        Key_ParaInit(btn);
    }
    // 有效点击判定：高电平时间足够长，且按压时间在有效范围内
    else if ((btn->high_cnt > JUDGE_TIME) && (btn->hold_cnt > CLICK_MIN_TIME && btn->hold_cnt < CLICK_MAX_TIME)) {
        if (btn->press_flag == 1) btn->buttonAction = BUTTON_SINGLE;
        else if (btn->press_flag == 2) btn->buttonAction = BUTTON_DOUBLE;
        else if (btn->press_flag == 3) btn->buttonAction = BUTTON_TRIPLE;
        Key_ParaInit(btn);
    }
}

// 动作：什么都不做 (用于纯状态流转)
void Action_None(buttonType *btn) {
    // Do nothing
}

void KEY_DEBUG(void) {
    // 使用常数而不是硬编码的数字
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        switch (button[i].buttonAction) {
            case BUTTON_SINGLE: {
                button[i].buttonAction = BUTTON_NULL;
                printf("%d->BUTTON_SINGLE\r\n", i);
                break;
            }
            case BUTTON_LONG_PRESS: {
                button[i].buttonAction = BUTTON_NULL;
                printf("%d->BUTTON_LONG_PRESS\r\n", i);
                break;
            }
            case BUTTON_DOUBLE: {
                button[i].buttonAction = BUTTON_NULL;
                printf("%d->BUTTON_DOUBLE\r\n", i);
                break;
            }
            case BUTTON_TRIPLE: {
                button[i].buttonAction = BUTTON_NULL;
                printf("%d->BUTTON_TRIPLE\r\n", i);
                break;
            }
            case BUTTON_NULL: {
                button[i].buttonAction = BUTTON_NULL;
                break;
            }
            default: {
                break;
            }
        }
    }
}