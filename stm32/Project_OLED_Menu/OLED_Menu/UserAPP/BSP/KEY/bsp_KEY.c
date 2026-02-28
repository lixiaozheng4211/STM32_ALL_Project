#include "bsp_KEY.h"

// ===================== 内部私有定义（外部不可见） =====================
// 单个按键的状态机状态
typedef enum {
  KEY_STATE_IDLE = 0, // 空闲状态（等待按下）
  KEY_STATE_DEBOUNCE_DOWN, // 消抖状态（检测到按下）
  KEY_STATE_PRESS_HOLD, // 按下保持状态（计时长按）
  KEY_STATE_DEBOUNCE_UP // 消抖状态（检测到松开）
} Key_State_t;

// 单个按键的控制结构体
typedef struct {
  GPIO_TypeDef *port; // GPIO端口
  uint16_t pin; // GPIO引脚
  Key_State_t state; // 状态机当前状态
  uint16_t count; // 计时器（单位：10ms）
} Key_Ctrl_t;

// ===================== 内部静态变量（外部不可见） =====================
static Key_Event_Callback_t s_user_callback = NULL; // 保存用户注册的回调函数

// 三个按键的硬件映射和控制数据
static Key_Ctrl_t s_keys[KEY_ID_MAX] = {
  // KEY1
  {
    .port = KEY1_PORT,
    .pin = KEY1_PIN,
    .state = KEY_STATE_IDLE,
    .count = 0,

  },
  // KEY2
  {
    .port = KEY2_PORT,
    .pin = KEY2_PIN,
    .state = KEY_STATE_IDLE,
    .count = 0,

  },
  // KEY3
  {
    .port = KEY3_PORT,
    .pin = KEY3_PIN,
    .state = KEY_STATE_IDLE,
    .count = 0,

  }
};

// ===================== 内部辅助函数（外部不可见） =====================
// 读取按键电平（上拉输入：按下=低电平GPIO_PIN_RESET，松开=高电平GPIO_PIN_SET）
static GPIO_PinState Key_Read_Level(Key_Ctrl_t *key) {
  return HAL_GPIO_ReadPin(key->port, key->pin);
}

// 触发用户回调函数（仅内部调用）
static void Key_Trigger_Callback(Key_ID_t id, Key_Event_t evt) {
  if (s_user_callback != NULL) {
    s_user_callback(id, evt);
  }
}

// ===================== 对外接口函数实现 =====================
void BSP_Key_Init(Key_Event_Callback_t callback) {
  // 保存用户回调函数
  s_user_callback = callback;

  // 初始化所有按键的状态机
  for (uint8_t i = 0; i < KEY_ID_MAX; i++) {
    s_keys[i].state = KEY_STATE_IDLE;
    s_keys[i].count = 0;
  }
}

void BSP_Key_Scan_IRQHandler(void) {
  for (uint8_t i = 0; i < KEY_ID_MAX; i++) {
    Key_Ctrl_t *key = &s_keys[i];
    GPIO_PinState level = Key_Read_Level(key);

    switch (key->state) {
      case KEY_STATE_IDLE:
        if (level == GPIO_PIN_RESET) {
          key->state = KEY_STATE_DEBOUNCE_DOWN;
          key->count = 0;
        }
        break;

      case KEY_STATE_DEBOUNCE_DOWN:
        key->count++;
        if (key->count >= KEY_DEBOUNCE_TIME) {
          if (level == GPIO_PIN_RESET) {
            key->state = KEY_STATE_PRESS_HOLD;
            key->count = 0;
            Key_Trigger_Callback(i, KEY_EVT_PRESSED); // 按下事件
          } else {
            key->state = KEY_STATE_IDLE;
          }
        }
        break;

      case KEY_STATE_PRESS_HOLD:
        // 检测松开
        if (level == GPIO_PIN_SET) {
          key->state = KEY_STATE_DEBOUNCE_UP;
          key->count = 0;
        }
        // 长按检测（可选）
        key->count++;
        if (key->count >= KEY_LONG_PRESS_TIME) {
          Key_Trigger_Callback(i, KEY_EVT_LONG_PRESSED);
          key->count = 0; // 防止重复触发
        }
        break;

      case KEY_STATE_DEBOUNCE_UP:
        key->count++;
        if (key->count >= KEY_DEBOUNCE_TIME) {
          if (level == GPIO_PIN_SET) {
            Key_Trigger_Callback(i, KEY_EVT_RELEASED); // 松开事件
            key->state = KEY_STATE_IDLE;
          } else {
            key->state = KEY_STATE_PRESS_HOLD;
          }
        }
        break;
    }
  }
}
