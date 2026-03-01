#include "key.h"

// ==================== 1. 定义事件与表结构 ====================
typedef enum {
  EVENT_NONE = 0,
  EVENT_PIN_FALLING, // 引脚下降沿 (按下)
  EVENT_PIN_RISING, // 引脚上升沿 (松开)
  EVENT_TICK // 定时器心跳 (时间流逝)
} ButtonEvent_t;

// 定义动作函数指针类型
typedef void (*ActionFunc_t)(buttonType *btn);

// 定义状态表的行结构
typedef struct {
  KeyActionType currentState;
  ButtonEvent_t triggerEvent;
  KeyActionType nextState;
  ActionFunc_t actionFunc;
} StateMachineRow_t;

// ==================== 2. 编写具体的动作函数 ====================

// 动作：记录按下瞬间
void Action_KeyDown(buttonType *btn);

// 动作：持续按压时的计时
void Action_Pressing(buttonType *btn);

// 动作：松开瞬间的处理
void Action_KeyUp(buttonType *btn);

// 动作：在 NULL 状态下熬时间并结算 (核心业务逻辑)
void Action_NullTick(buttonType *btn);

// 动作：什么都不做 (用于纯状态流转)
void Action_None(buttonType *btn);


#define TABLE_SIZE (sizeof(button_state_table)/sizeof(button_state_table[0]))

// ==================== 4. 事件获取器 ====================
// 把硬件电平和状态机剥离开来
ButtonEvent_t Get_Button_Event(buttonType *btn);

// ==================== 5. 极简的核心引擎 ====================
// 以后不管加多少个状态，这个函数永远都不用改了
void Key_Scan_Engine(buttonType *btn);

void KEY_DEBUG(void);