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
#include "i2c.h"
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
#define TIM3_COUNT_FREQ 10000  // TIM3计数频率：10kHz（匹配tim.c的PSC配置）
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t Freq = 0;            // 初始化频率为0，避免随机值
uint16_t overflow_cnt = 0;    // TIM3溢出次数（关键：解决低频捕获）
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// TIM3输入捕获中断回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef * htim) {
  if (htim->Instance == TIM3) {
    // 读取捕获值（通道1）
    uint16_t capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    // 计算总计数（含溢出），+1修正0开始计数的误差
    uint32_t total_capture = (uint32_t)capture + (uint32_t)overflow_cnt * 65536U + 1;

    // 除0保护：避免程序卡死
    if (total_capture != 0) {
      Freq = TIM3_COUNT_FREQ / total_capture;
    } else {
      Freq = 0;
    }

    // 重置溢出计数，准备下一次捕获
    overflow_cnt = 0;
    // 清除捕获标志，确保下次捕获正常
    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC1);
  }
}

// TIM3溢出（更新）中断回调函数（必须加！）
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM3) {
    overflow_cnt++;  // 溢出次数+1
    // 防止溢出次数过多导致数值溢出（可选）
    if (overflow_cnt > 100) {
      overflow_cnt = 0;
      Freq = 0;
    }
  }
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
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_Clear();  // 清屏：避免残留显示

  // 启动TIM2_CH1 PWM输出（有信号才能捕获）
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  // 启动TIM3_CH1输入捕获中断 + TIM3更新（溢出）中断（双中断都要启动！）
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim3);

  // OLED初始化显示
  OLED_ShowString(1,1,"Freq:");
  OLED_ShowString(2,1,"Duty:");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t duty_dir = 1;  // 占空比调整方向：1=递增，0=递减
  uint32_t duty_val = 50;// 初始占空比50（确保有PWM信号输出）
  while (1)
  {
    /* USER CODE END WHILE */
    // 动态调整PWM占空比（0→99→0循环，确保有信号输出）
    if (duty_dir) {
      duty_val++;
      if (duty_val >= 99) {
        duty_dir = 0;
      }
    } else {
      duty_val--;
      if (duty_val <= 0) {
        duty_dir = 1;
      }
    }
    // 设置TIM2_CH1占空比（关键：输出有效PWM信号）
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty_val);

    // OLED显示频率（5位）和占空比（2位）
    OLED_ShowNum(1,6,Freq,5);
    OLED_ShowNum(2,6,duty_val,2);

    HAL_Delay(10);  // 调整刷新率，避免OLED刷屏过快
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