#include "BSP_UltraSound.h"
#include "tim.h" // 引入定时器 (假设你用的是 htim4)

float System_Distance_cm = 0.0f;

void BSP_Sonar_Trig(void) {
  HAL_GPIO_WritePin(UltraSound_Trig_GPIO_Port, UltraSound_Trig_Pin, GPIO_PIN_SET);
  // 空指令循环 -- 常见延迟操作
  for (volatile int i = 0; i < 72; i++) { __NOP(); }
  HAL_GPIO_WritePin(UltraSound_Trig_GPIO_Port, UltraSound_Trig_Pin, GPIO_PIN_RESET);
  // 开启双通道的IT中断
  HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);
}

// 捕获完成中断：当 Echo 脉冲结束（下降沿）时，硬件会自动跳到这里
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM4) {
    // 在 PWM Input 模式下，CH2 通常负责抓取脉宽（下降沿）
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
      uint32_t echo_time_us = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

      // 2. 转换为距离
      System_Distance_cm = echo_time_us * 0.017f; // 不应该是计数器CNT

      // 3. 抓完一次就立刻关掉中断，防止环境噪音乱触发，等下一次 Trig 再开
      HAL_TIM_IC_Stop_IT(htim, TIM_CHANNEL_1);
      HAL_TIM_IC_Stop_IT(htim, TIM_CHANNEL_2);
    }
  }
}

// 溢出中断 (防卡死补丁)：如果超声波对着天空打，永远没回波怎么办？
// 前提：你在 CubeMX 里给 TIM4 开启了 Update Interrupt (更新中断) 这个中断是固定的不管是谁让CNT溢出都会触发中断
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  // 只要跑到这里，说明定时器计满了 65535 微秒 (约 65 毫秒还没收到回波)
  if (htim->Instance == TIM4) {
    System_Distance_cm = 999.0f; // 报告一个假数值，代表超出量程或错误

    // 强行关闭捕获，防止系统死等
    HAL_TIM_IC_Stop_IT(htim, TIM_CHANNEL_1);
    HAL_TIM_IC_Stop_IT(htim, TIM_CHANNEL_2);
  }
}