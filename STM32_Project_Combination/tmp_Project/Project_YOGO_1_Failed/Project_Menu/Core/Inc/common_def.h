#ifndef __COMMON_DEF_H
#define __COMMON_DEF_H

#include "stm32f1xx_hal.h"

// ================= 旋转编码器事件枚举 =================
typedef enum {
  ENCODER_NONE,        // 无操作
  ENCODER_CW,          // 顺时针旋转
  ENCODER_CCW,         // 逆时针旋转
  ENCODER_PRESS,       // 按键按下
  ENCODER_LONG_PRESS   // 长按（用于返回）
} Encoder_Event_t;

// ================= 菜单状态枚举 =================
typedef enum {
  MENU_STATE_MAIN,     // 主菜单
  MENU_STATE_RUNNING   // 运行模块
} Menu_State_t;

// ================= 模块接口结构体（插件核心） =================
typedef struct {
  const char* Name;          // 菜单显示名称
  void (*Init)(void);        // 模块初始化
  void (*Run)(void);         // 模块运行（循环调用）
  void (*Exit)(void);        // 模块退出
} Module_Interface_t;

#endif