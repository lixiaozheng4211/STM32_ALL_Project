/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../app.h"
#include "../bsp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// ===================== 第一步：定义你的功能函数 =====================
// 这些函数会在菜单被选中时执行
void Func_StartMotor(void)
{
  OLED_ShowString(4, 1, "Motor Start!");
}

void Func_StopMotor(void)
{
  OLED_ShowString(4, 1, "Motor Stop!");
}

void Func_Settings(void)
{
  OLED_ShowString(4, 1, "Settings...");
}

// ===================== 第二步：定义你的菜单树结构 =====================
// 注意：要从最底层的子菜单开始定义，最后定义根菜单

// 1. 定义“运动控制”子菜单
const Menu_Item_t menu_motor[] = {
  {"start motor", Func_StartMotor, NULL, 0},   // 可执行项
  {"stop motor", Func_StopMotor,  NULL, 0},   // 可执行项
};

// 2. 定义“系统设置”子菜单
const Menu_Item_t menu_settings[] = {
  {"param set", Func_Settings, NULL, 0},
  {"save set", NULL, NULL, 0}, // 预留
};

// 3. 定义根菜单（主菜单）
const Menu_Item_t menu_root[] = {
  {"motion control", NULL, menu_motor, 2},       // 文件夹：有子菜单
  {"system set", NULL, menu_settings, 2},    // 文件夹
  {"about us", NULL, NULL, 0},              // 预留
};

// ===================== 第三步：显示层刷新函数 =====================
// 这是连接菜单逻辑和OLED的桥梁
void Menu_Display_Refresh(void)
{
  const Menu_Item_t *parent = App_Menu_Get_Current_Parent();
  uint8_t cursor = App_Menu_Get_Cursor();

  // 清屏
  OLED_Clear();

  // 遍历显示当前页的所有菜单项
  // 0.96寸OLED最多显示4行8x16字符
  for(uint8_t i=0; i<parent->child_count && i<4; i++)
  {
    if(i == cursor)
    {
      // 当前光标行：显示箭头
      OLED_ShowChar(i+1, 1, '>');
    }
    // 显示菜单名称
    OLED_ShowString(i+1, 2, (char*)parent[i].name);
  }
}

// ===================== 第四步：按键事件回调 =====================
void My_Key_Callback(Key_ID_t id, Key_Event_t evt)
{
  // 1. 把短按事件传给菜单逻辑
  App_Menu_Input_Key(id, evt);

  // 2. 处理长按事件（比如长按KEY1返回主页）
  if(evt == KEY_EVT_LONG_PRESS)
  {
    if(id == KEY_ID_1)
    {
      App_Menu_Jump_To(menu_root); // 跳回根菜单
    }
  }

  // 3. 只要有按键事件，就刷新屏幕
  Menu_Display_Refresh();
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  /* USER CODE END 2 */
  // 2. BSP层初始化（顺序：I2C -> OLED -> Key）
  Soft_I2C_Init();
  OLED_Init();
  BSP_Key_Init(My_Key_Callback);

  // 3. APP层初始化（菜单）
  App_Menu_Init(menu_root);

  // 4. 启动定时器
  HAL_TIM_Base_Start_IT(&htim2);

  // 5. 初始显示
  Menu_Display_Refresh();
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
