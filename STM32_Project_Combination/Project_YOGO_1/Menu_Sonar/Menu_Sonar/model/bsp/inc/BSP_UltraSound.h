#ifndef BSP_UltraSound_h
#define BSP_UltraSound_h
#include "main.h"
extern float System_Distance_cm;
void BSP_Sonar_Trig(void);
// 捕获完成中断：当 Echo 脉冲结束（下降沿）时，硬件会自动跳到这里
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) ;
// 溢出中断 (防卡死补丁)：如果超声波对着天空打，永远没回波怎么办？
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) ;
#endif //BSP_UltraSound_h