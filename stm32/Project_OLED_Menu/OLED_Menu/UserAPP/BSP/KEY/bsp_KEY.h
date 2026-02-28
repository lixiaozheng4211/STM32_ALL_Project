#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

  // ===================== 硬件配置区：换引脚只改这里！！！ =====================
  // 你的实际引脚定义
#define KEY1_PORT    GPIOA
#define KEY1_PIN     GPIO_PIN_6
#define KEY2_PORT    GPIOB
#define KEY2_PIN     GPIO_PIN_1
#define KEY3_PORT    GPIOB
#define KEY3_PIN     GPIO_PIN_11

  // ===================== 参数配置区：调手感只改这里！！！ =====================
#define KEY_DEBOUNCE_TIME    2   // 消抖时间：2*10ms=20ms
#define KEY_LONG_PRESS_TIME  100 // 长按阈值：100*10ms=1s
  // ============================================================================

  // ===================== 对外枚举定义 =====================
  // 物理按键ID
  typedef enum {
    KEY_ID_1 = 0,
    KEY_ID_2,
    KEY_ID_3,
    KEY_ID_MAX // 按键总数，不要修改
} Key_ID_t;

  // 按键事件
  typedef enum {
    KEY_EVT_NONE = 0,
    KEY_EVT_SHORT_PRESS, // 短按（松开触发）
    KEY_EVT_LONG_PRESS,  // 长按（按下达到阈值触发）
    KEY_EVT_RELEASED     // 松开（仅用于特殊逻辑）
} Key_Event_t;

  // ===================== 回调函数类型定义（解耦核心） =====================
  // 当按键事件发生时，驱动会调用这个函数，把事件传递给上层
  typedef void (*Key_Event_Callback_t)(Key_ID_t id, Key_Event_t evt);

  // ===================== 对外接口函数声明 =====================
  /**
   * @brief  按键驱动初始化
   * @param  callback 上层注册的事件回调函数
   * @retval 无
   */
  void BSP_Key_Init(Key_Event_Callback_t callback);

  /**
   * @brief  按键扫描函数（必须在定时器中断里调用，推荐10ms一次）
   * @param  无
   * @retval 无
   */
  void BSP_Key_Scan_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif // __BSP_KEY_H