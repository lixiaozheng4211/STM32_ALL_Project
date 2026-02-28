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
#include "OLED.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
uint32_t UpEdge = 0;
uint32_t DownEdge = 0;
float Distance = 0;
uint8_t capture_complete_flag = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  OLED_ShowString(1,1,"CB:123");  // 回调触发标记

  if (htim->Instance == TIM1) {
    // 1. 捕获CH3（上升沿）
    if (htim->Channel == TIM_CHANNEL_3) {
      UpEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
      __HAL_TIM_CLEAR_FLAG(htim,TIM_FLAG_CC3);
      __HAL_TIM_CLEAR_IT(htim,TIM_IT_CC3);
    }
    // 2. 捕获CH4（下降沿）
    if (htim->Channel == TIM_CHANNEL_4) {
      DownEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
      __HAL_TIM_CLEAR_FLAG(htim,TIM_FLAG_CC4);
      __HAL_TIM_CLEAR_IT(htim,TIM_IT_CC4);
      capture_complete_flag = 1;
    }

    // 3. 计算距离（保留逻辑）
    if (capture_complete_flag == 1) {
      uint32_t pulse_width = (DownEdge >= UpEdge) ? (DownEdge - UpEdge) : (65535 - UpEdge + DownEdge);
      Distance = pulse_width * 0.034f / 2.0f;
      capture_complete_flag = 0;
    }
  }

  // 4. 显示原始捕获值（重点！看是否变化）
  OLED_ShowString(3,1,"Up:");
  OLED_ShowNum(3,4, UpEdge, 4);        // 显示上升沿计数值
  OLED_ShowString(4,1,"Down:");
  OLED_ShowNum(4,4, DownEdge, 4);      // 显示下降沿计数值
  OLED_ShowNum(2,1,(uint32_t)Distance,3);
  OLED_ShowString(2,4,".");
  OLED_ShowNum(2,5,(uint32_t)(Distance*10)%10,1);

  OLED_Clear();
}

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
  MX_TIM1_Init();
  HAL_Delay(20);
  OLED_Init();
  /* USER CODE BEGIN 2 */
  // 主要是为了防止异常
  __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC3);  // 清除CH3捕获标志
  __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_CC3);      // 清除CH3中断挂起位
  __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC4);  // 清除CH4捕获标志
  __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_CC4);      // 清除CH4中断挂起位

  // 2 启动TIM1的基础计数（MX_TIM1_Init仅初始化，未启动计数，必须手动启动）
  HAL_TIM_Base_Start(&htim1);

  // 3 开启CH3（Echo）和CH4（间接捕获）的「带中断」输入捕获（核心！）
  // CH3：直接捕获Echo的上升沿，开启中断
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);
  // CH4：间接捕获Echo的下降沿，开启中断
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_4);
  /* USER CODE END 2 */
  OLED_ShowString(1,1,"Distance:");
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    // 1. 触发前清零TIM1计数器，保证每次测距的计数基准从0开始（核心！）
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
    HAL_Delay(2);
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
