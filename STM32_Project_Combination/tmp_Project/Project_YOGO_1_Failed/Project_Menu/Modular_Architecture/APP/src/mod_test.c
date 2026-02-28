#include "mod_test.h"
#include "OLED.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"

// 模块内部变量
static uint8_t g_TestCnt = 0;

// 模块初始化
static void Test_Init(void)
{
  g_TestCnt = 0;
  OLED_Init();
  OLED_ShowString(0, 0, "Test Module Init");
  HAL_Delay(500);
}

// 模块运行
static void Test_Run(void)
{
  char buf[20];
  OLED_Clear();
  OLED_ShowString(0, 0, "Test Module Run");
  sprintf(buf, "Cnt: %d", g_TestCnt++);
  OLED_ShowString(0, 2, buf);
  OLED_ShowString(0, 6, "Long Press Back");
}

// 模块退出
static void Test_Exit(void)
{
  OLED_Clear();
  OLED_ShowString(0, 0, "Test Module Exit");
  HAL_Delay(500);
}

// 注册模块（插件接口）
Module_Interface_t Mod_Test = {
  .Name = "Test Module",
  .Init = Test_Init,
  .Run  = Test_Run,
  .Exit = Test_Exit,
};