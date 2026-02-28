#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H

#include "main.h" // 包含底层硬件定义

typedef enum {
  ENC_EVENT_NONE = 0,     // 无动作
  ENC_EVENT_UP,           // 正转 (向上/向右)
  ENC_EVENT_DOWN,         // 反转 (向下/向左)
  ENC_EVENT_SHORT_PRESS,  // 短按
  ENC_EVENT_LONG_PRESS    // 长按
} Encoder_Event_t;


void BSP_Encoder_Init(void);
Encoder_Event_t BSP_Encoder_Scan(void);

#endif /* __BSP_ENCODER_H */