#include "app.h"
#include "encoder.h"
#include "OLED.h"
#include "stm32f1xx_hal.h"


// ================= 模块列表（插件注册） =================
static Module_Interface_t* Module_List[] = {
    &Mod_Test,
    // &Mod_MPU6050,  // 后续添加模块只需在这里注册
    // &Mod_Buzzer,
};
#define MODULE_CNT  (sizeof(Module_List) / sizeof(Module_List[0]))
#define OLED_MAX_ROW 4  // OLED最大行数（从1开始）
#define OLED_MAX_COL 16 // OLED最大列数（从1开始）
#define MENU_TITLE_ROW 1// 菜单标题行
#define MENU_ITEM_START_ROW 2 // 菜单项起始行

// ================= 静态变量 =================
static Menu_State_t g_MenuState = MENU_STATE_MAIN; // 当前菜单状态
static uint8_t g_SelectedIndex = 0;                // 当前选中索引
static uint8_t g_RunningModuleIndex = 0;           // 运行中的模块索引

// ================= 静态函数：绘制主菜单 =================
static void Menu_Draw_Main(void)
{
    OLED_Clear(); // 清空屏幕

    // 绘制菜单标题（适配16列，避免超界）
    OLED_ShowString(1, MENU_TITLE_ROW, "Main Menu"); // 列1，行1，长度8<16

    // 计算可显示的菜单项数量（标题占1行，剩余3行显示菜单项）
    uint8_t showable_item_cnt = OLED_MAX_ROW - MENU_TITLE_ROW;
    for(uint8_t i = 0; i < MODULE_CNT && i < showable_item_cnt; i++)
    {
        uint8_t current_row = MENU_ITEM_START_ROW + i; // 菜单项行：2/3/4
        if(current_row > OLED_MAX_ROW) break; // 防止行号超出OLED最大行数

        if(i == g_SelectedIndex)
        {
            OLED_ShowString(1, current_row, ">"); // 选中光标：列1，对应行
        }
        // 菜单项名称从列3开始显示，避免和光标重叠
        OLED_ShowString(3, current_row, (char*)Module_List[i]->Name);
    }
}

// ================= 静态函数：处理编码器事件 =================
static void Menu_Process_Event(Encoder_Event_t event)
{
    switch(g_MenuState)
    {
        // 主菜单状态
        case MENU_STATE_MAIN:
            switch(event)
            {
                case ENCODER_CW: // 顺时针：下选
                    // 限制选中索引不超过可显示的最大项数
                    if(g_SelectedIndex < (MODULE_CNT - 1) &&
                       g_SelectedIndex < (OLED_MAX_ROW - MENU_TITLE_ROW - 1))
                        g_SelectedIndex++;
                    break;
                case ENCODER_CCW: // 逆时针：上选
                    if(g_SelectedIndex > 0)
                        g_SelectedIndex--;
                    break;
                case ENCODER_PRESS: // 按下：进入模块
                    g_RunningModuleIndex = g_SelectedIndex;
                    Module_List[g_RunningModuleIndex]->Init(); // 模块初始化
                    g_MenuState = MENU_STATE_RUNNING;         // 切换状态
                    break;
            }
            break;

        // 运行模块状态
        case MENU_STATE_RUNNING:
            if(event == ENCODER_LONG_PRESS) // 长按：返回主菜单
            {
                Module_List[g_RunningModuleIndex]->Exit(); // 模块退出
                g_MenuState = MENU_STATE_MAIN;            // 切回主菜单
                g_SelectedIndex = 0;                      // 重置选中索引
            }
            break;
    }
}

// ================= 应用初始化 =================
void App_Init(void)
{
    // 1. 初始化硬件（仅保留必要的编码器+OLED）
    Encoder_Init();  // 编码器初始化（轮询方式，无需中断）
    OLED_Init();     // OLED初始化（适配1开始的行列）

    // 【关键修改】移除TIM2中断启动（轮询方式不需要）
    // HAL_TIM_Base_Start_IT(&htim2); // 删掉这行！

    // 2. 初始化提示（适配1开始的行列，不超16列）
    OLED_Clear();
    OLED_ShowString(1, 2, "System Init OK!"); // 列1，行2（居中更友好）
    HAL_Delay(1000);
}

// ================= 应用主循环（核心：轮询读取编码器） =================
void App_Run(void)
{
    // 1. 【轮询核心】读取编码器事件（无中断，纯主循环检测）
    Encoder_Event_t event = Encoder_Get_Event();

    // 2. 处理事件（状态机核心）
    if(event != ENCODER_NONE)
    {
        Menu_Process_Event(event);
    }

    // 3. 状态执行
    switch(g_MenuState)
    {
        case MENU_STATE_MAIN:
            Menu_Draw_Main(); // 绘制主菜单
            break;

        case MENU_STATE_RUNNING:
            Module_List[g_RunningModuleIndex]->Run(); // 运行模块
            break;
    }

    HAL_Delay(20); // 防抖+控制轮询频率（关键：避免CPU空转）
}