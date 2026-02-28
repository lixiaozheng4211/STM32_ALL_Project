#include "Encoder.h"
#include "tim.h"

// 编码器初始化：开启TIM3编码器模式
void Encoder_Init(void)
{
  // 开启TIM3所有通道的编码器模式（CubeMX已配置好参数，直接启动即可）
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  // 初始化计数器为0，避免上电初始值异常
  __HAL_TIM_SET_COUNTER(&htim3, 0);
}

// 读取编码器计数值：和标准库功能一致，int16_t解决反转溢出
int16_t Encoder_Get(void)
{
  int16_t temp;
  temp = __HAL_TIM_GET_COUNTER(&htim3); // 读取TIM3计数值（有符号，反转负）
  __HAL_TIM_SET_COUNTER(&htim3, 0);     // 清零计数器，为下次采样做准备
  return temp;
}