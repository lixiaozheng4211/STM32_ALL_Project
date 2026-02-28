#ifndef __ENCODER_H
#define __ENCODER_H

#include "common_def.h"
#include "stm32f1xx_hal.h"

// ================= 硬件配置（与CubeMX一致） =================
#define ENCODER_A_GPIO_PORT    GPIOA
#define ENCODER_A_GPIO_PIN     GPIO_PIN_0
#define ENCODER_B_GPIO_PORT    GPIOA
#define ENCODER_B_GPIO_PIN     GPIO_PIN_2
#define ENCODER_KEY_GPIO_PORT  GPIOA
#define ENCODER_KEY_GPIO_PIN   GPIO_PIN_1

// ================= 函数声明 =================
void Encoder_Init(void);                  // 编码器初始化
Encoder_Event_t Encoder_Get_Event(void);  // 获取编码器事件
void Encoder_TIM2_Callback(void);         // TIM2中断回调（供stm32f1xx_it.c调用）

#endif