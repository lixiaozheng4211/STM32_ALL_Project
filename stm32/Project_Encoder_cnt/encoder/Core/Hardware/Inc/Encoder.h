#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f1xx_hal.h"

// 声明TIM3句柄（CubeMX自动生成，在tim.h中）
extern TIM_HandleTypeDef htim3;

// 编码器初始化（实际是开启编码器，CubeMX已完成底层初始化）
void Encoder_Init(void);
// 读取编码器计数值：读值后清零，返回int16_t有符号数（正=正转，负=反转）
int16_t Encoder_Get(void);

#endif