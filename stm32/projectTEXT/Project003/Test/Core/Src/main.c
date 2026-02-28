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

#include <stdio.h>

#include "tim.h"
#include "gpio.h"
#include "OLED.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// 微秒延时函数（F103C8T6 72MHz专用）
void HAL_Delay_us(uint32_t us) {
    us *= (SystemCoreClock / 1000000) / 8; // 72MHz下，1us≈9个NOP
    while (us--) { __NOP(); }
}
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
uint32_t UpEdge = 0;    // 上升沿捕获值（CH3）
uint32_t DownEdge = 0;  // 下降沿捕获值（CH4间接）
float distance = 0.0f;  // 测距结果（cm，必须用float）
uint8_t capture_flag = 0; // 捕获完成标志：1=已捕获上升+下降沿
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// TIM1输入捕获回调函数（适配直接/间接捕获模式）
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) { // TIM句柄类型定义
  if (htim->Instance == TIM1) {
    // 通道3捕获上升沿（直接模式）
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
      UpEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
      __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC3);    // 清CH3标志
      __HAL_TIM_CLEAR_IT(htim, TIM_IT_CC3);        // 清CH3中断挂起位
    }

    // 通道4捕获下降沿（间接模式，复用CH3信号）
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
      DownEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
      __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC4);    // 清CH4标志
      __HAL_TIM_CLEAR_IT(htim, TIM_IT_CC4);        // 清CH4中断挂起位 zhe
      capture_flag = 1; // 标记：上升+下降沿都已捕获
    }

    // 仅当捕获完成时计算距离
    if (capture_flag == 1) {
      uint32_t pulse_width = 0;
      // 处理计数器溢出（TIM1周期65535，1MHz计数）
      if (DownEdge >= UpEdge) {
        pulse_width = DownEdge - UpEdge;
      } else {
        pulse_width = (65535 - UpEdge) + DownEdge;
      }
      // 核心公式：距离(cm) = 脉冲宽度(us) × 0.034 / 2
      distance = pulse_width * 0.034f / 2.0f;
      capture_flag = 0; // 重置标志，准备下一次捕获
    }
  }
}

// 函数定义：HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
// 英文拆解：
// HAL_ = 硬件抽象层（统一封装底层硬件操作）
// TIM_ = 定时器（指定操作的外设是定时器）
// IC_ = Input Capture（输入捕获，模块子功能）
// Capture = 捕获（核心动作）
// Callback = 回调（函数类型：中断触发后自动调用）
// 伪代码翻译：定义一个“定时器输入捕获中断回调函数”，接收“定时器句柄”作为参数
// void 定时器输入捕获中断回调函数(定时器句柄 *定时器句柄指针) {
//
//   // 第一层判断：if (htim->Instance == TIM1)
//   // 英文拆解：
//   // Instance = 实例（句柄里的成员，指向具体的定时器外设，如TIM1/TIM2）
//   // 伪代码翻译：如果触发中断的定时器是TIM1（过滤其他定时器的中断）
//   如果 (定时器句柄指针->具体定时器实例 == TIM1) {
//
//     // 处理CH3上升沿捕获：if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
//     // 英文拆解：
//     // Channel = 通道（定时器的输入/输出通道）
//     // HAL_TIM_ACTIVE_CHANNEL_3 = 激活的通道3（HAL库定义的通道3常量）
//     // 伪代码翻译：如果本次中断是TIM1的通道3触发的（上升沿捕获）
//     如果 (定时器句柄指针->触发中断的通道 == 定时器通道3) {
//
//       // UpEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3)
//       // 英文拆解：
//       // Read = 读取（动作）
//       // Captured = 捕获的（形容词，修饰Value）
//       // Value = 值（操作对象：捕获寄存器里的计数值）
//       // TIM_CHANNEL_3 = 定时器通道3（指定读取的通道）
//       // 伪代码翻译：读取TIM1通道3捕获寄存器的值，赋值给“上升沿计数值”变量
//       上升沿计数值 = 读取定时器捕获值(定时器句柄指针, 定时器通道3);
//
//       // __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC3)
//       // 英文拆解：
//       // __ = 底层宏（直接操作寄存器，无HAL库封装）
//       // Clear = 清除（核心动作）
//       // Flag = 标志（操作对象：外设层的标志位）
//       // TIM_FLAG_CC3 = 定时器通道3捕获标志（CC=Capture Compare，捕获比较）
//       // 伪代码翻译：清除TIM1通道3的“捕获标志位”（避免标志残留导致重复中断）
//       清除定时器标志位(定时器句柄指针, TIM1通道3捕获标志);
//
//       // __HAL_TIM_CLEAR_IT(htim, TIM_IT_CC3)
//       // 英文拆解：
//       // IT = Interrupt（中断，缩写）
//       // TIM_IT_CC3 = 定时器通道3捕获中断（操作对象：内核层的中断挂起位）
//       // 伪代码翻译：清除TIM1通道3的“中断挂起位”（彻底结束本次中断请求）
//       清除定时器中断挂起位(定时器句柄指针, TIM1通道3捕获中断);
//     }
//
//     // 处理CH4下降沿捕获：if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
//     // 伪代码翻译：如果本次中断是TIM1的通道4触发的（下降沿捕获）
//     如果 (定时器句柄指针->触发中断的通道 == 定时器通道4) {
//
//       // DownEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4)
//       // 伪代码翻译：读取TIM1通道4捕获寄存器的值，赋值给“下降沿计数值”变量
//       下降沿计数值 = 读取定时器捕获值(定时器句柄指针, 定时器通道4);
//
//       // __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC4)
//       // 伪代码翻译：清除TIM1通道4的“捕获标志位”
//       清除定时器标志位(定时器句柄指针, TIM1通道4捕获标志);
//
//       // __HAL_TIM_CLEAR_IT(htim, TIM_IT_CC4)
//       // 伪代码翻译：清除TIM1通道4的“中断挂起位”
//       清除定时器中断挂起位(定时器句柄指针, TIM1通道4捕获中断);
//
//       // capture_flag = 1
//       // 伪代码翻译：设置“捕获完成标志”为1（表示上升沿+下降沿都已捕获）
//       捕获完成标志 = 1;
//     }
//
//     // 计算距离：if (capture_flag == 1)
//     // 伪代码翻译：如果捕获完成（上升+下降沿都已获取）
//     如果 (捕获完成标志 == 1) {
//
//       // 定义脉冲宽度变量
//       脉冲宽度 = 0;
//
//       // 处理计数器溢出：if (DownEdge >= UpEdge)
//       // 伪代码翻译：如果下降沿计数值 ≥ 上升沿计数值（无溢出）
//       如果 (下降沿计数值 >= 上升沿计数值) {
//         // 脉冲宽度 = 下降沿计数值 - 上升沿计数值
//         脉冲宽度 = 下降沿计数值 - 上升沿计数值;
//       } 否则 {
//         // 溢出情况：脉冲宽度 = (65535 - 上升沿计数值) + 下降沿计数值
//         脉冲宽度 = (65535 - 上升沿计数值) + 下降沿计数值;
//       }
//
//       // 距离计算：distance = pulse_width * 0.034f / 2.0f
//       // 伪代码翻译：距离(cm) = 脉冲宽度(us) × 0.034（声速） / 2（往返距离）
//       距离 = 脉冲宽度 * 0.034f / 2.0f;
//
//       // 重置捕获完成标志
//       捕获完成标志 = 0;
//     }
//   }
// }
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
  MX_TIM1_Init();  // TIM1初始化（包含直接/间接捕获配置）
  /* USER CODE BEGIN 2 */
  // 启动TIM1基础计数
  HAL_TIM_Base_Start(&htim1);
  // 启动CH3（直接）和CH4（间接）的带中断捕获
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_4);

  // 初始化OLED
  OLED_Init();
  OLED_Clear(); // 清屏
  OLED_ShowString(1,1,"Distance:");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    // ========== 规范的Trig触发逻辑 ==========
    __HAL_TIM_SET_COUNTER(&htim1, 0);  // 触发前清零计数器（关键）
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);  // Trig拉高
    HAL_Delay_us(15);  // 精准15us高电平（满足模块≥10us要求）
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET); // Trig拉低

    HAL_Delay(20);    // 测距间隔（避免连续触发干扰）
    // OLED显示：保留1位小数，适配float类型
    OLED_ShowNum(1, 10, (uint32_t)distance, 3);          // 整数部分
    OLED_ShowChar(1, 13, '.');                           // 小数点
    OLED_ShowNum(1, 14, (uint32_t)(distance*10)%10, 1);  // 小数部分
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