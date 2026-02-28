/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "OLED.h"
#include "Encoder.h"// 引入编码器驱动

/* 全局变量：存储编码器速度（int16_t，正=正转，负=反转，和标准库一致）*/
int16_t Speed = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
int main(void)
{
  /* 1. HAL库初始化：系统时钟、中断、GPIO等（CubeMX自动生成）*/
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();  // TIM2定时中断初始化（CubeMX自动生成）
  MX_TIM3_Init();  // TIM3编码器初始化（CubeMX自动生成）

  /* 2. 外设初始化：OLED + 编码器 */
  OLED_Init();     // 初始化OLED
  Encoder_Init();  // 初始化编码器（开启TIM3编码器模式）

  /* 3. 开启TIM2定时中断模式（核心！HAL库开启中断的关键API）*/
  HAL_TIM_Base_Start_IT(&htim2);

  /* 4. OLED显示初始化：和标准库一致 */
  OLED_Clear();
  OLED_ShowString(1, 1, "Speed:");

  /* 主循环：只做OLED显示，无其他耗时操作 */
  while (1)
  {
    // 第1行第7列显示有符号速度值，占5位（和标准库一致）
    OLED_ShowSignedNum(1, 7, Speed, 5);
  }
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

/* TIM2定时更新中断回调函数：定时时间到（10ms）自动执行 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * const htim) {
  // 判制定时器实例：确保是TIM2的中断（规范写法，避免多个定时器中断冲突）
  if(htim->Instance == TIM2) {
    // 核心操作：读取编码器计数值，赋值给全局速度变量（和标准库中断里的操作一致）
    Speed = Encoder_Get();
    // HAL库会自动清除TIM2的更新中断标志位，无需手动清除（比标准库少一步）
  }
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
