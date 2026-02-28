#ifndef __APP_H
#define __APP_H

#include "common_def.h"
#include "oled.h" // 你的OLED HAL驱动头文件

// ================= 函数声明 =================
void App_Init(void);    // 应用初始化
void App_Run(void);     // 应用主循环（放在while(1)）

// ================= 模块声明（插件注册） =================
extern Module_Interface_t Mod_Test; // 测试模块

#endif