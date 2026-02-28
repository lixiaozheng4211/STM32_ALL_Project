/**
 * main_example.c
 * 集成示例 —— 展示如何在 CubeMX 生成的 main.c 中接入菜单系统
 * 请将以下代码片段复制到 main.c 对应的 USER CODE 区域
 */

/* ---- USER CODE BEGIN Includes ---- */
#include "OLED.h"
#include "bsp.h"
#include "app.h"
/* ---- USER CODE END Includes ---- */

/* ---- USER CODE BEGIN 0 ---- */

/* 动作函数示例 */
void Func_LED_ON(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

void Func_LED_OFF(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void Func_Settings(void)
{
    /* 自定义功能 */
}

/* 菜单定义 */
const Menu_Item_t menu_led[] = {
    {"LED ON",  Func_LED_ON,  NULL, 0},
    {"LED OFF", Func_LED_OFF, NULL, 0},
};

const Menu_Item_t menu_root[] = {
    {"LED Ctrl", NULL, menu_led, 2},
    {"Settings", Func_Settings, NULL, 0},
};

#define ROOT_COUNT  (sizeof(menu_root) / sizeof(menu_root[0]))

/* ---- USER CODE END 0 ---- */

/*
 * ---- USER CODE BEGIN 2 (在 main 函数中，外设初始化之后) ----
 *
 *   BSP_Encoder_Init(&htim3);
 *   HAL_TIM_Base_Start_IT(&htim2);
 *   OLED_Init();
 *   App_Menu_Init(menu_root, ROOT_COUNT);
 *
 * ---- USER CODE END 2 ----
 */

/*
 * ---- USER CODE BEGIN 3 (主循环 while(1) 内部) ----
 *
 *   Encoder_Event_t evt = BSP_Encoder_GetEvent();
 *   if (evt != ENC_EVT_NONE) {
 *       App_Menu_Process(evt);
 *   }
 *   App_Menu_UI_Draw();
 *
 * ---- USER CODE END 3 ----
 */

/* ---- USER CODE BEGIN 4 (main.c 末尾，函数定义区) ---- */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        BSP_Encoder_Scan();
    }
}
/* ---- USER CODE END 4 ---- */
