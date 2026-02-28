/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : TIM1双通道超声波测距（修正版）
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <math.h>
#include "tim.h"
#include "gpio.h"
#include "OLED.h"

/* Private define ------------------------------------------------------------*/
#define TIM1_CLK_FREQ  1000000UL  // TIM1计数频率（1MHz=1us/次）
#define TRIG_PULSE_US  15         // 触发脉冲宽度（15μs，确保触发）
#define CAPTURE_TIMEOUT 100       // 捕获超时时间（100ms，对应~17米）
/* USER CODE END PD */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Delay_us(uint32_t us);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief  微秒级延时（适配72MHz主频）
  * @param  us: 延时微秒数
  * @retval None
  */
void Delay_us(uint32_t us)
{
  uint32_t delay = us;  // 简化逻辑，1MHz下直接等于us数
  while(delay--){__NOP();}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU初始化 */
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM1_Init();

  /* USER CODE BEGIN 2 */
  OLED_Init();
  HAL_TIM_Base_Start(&htim1);  // 关键：启动TIM1计数器（必须！）
  /* USER CODE END 2 */

  /* 无限循环 */
  while (1) {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    // 清除捕获标志位
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC1);
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC2);

    // 启动输入捕获（CH1=上升沿，CH2=下降沿）
    HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_2);

    // 发送触发脉冲（PA0）
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    Delay_us(TRIG_PULSE_US);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

    // 等待捕获完成（超时100ms）
    uint32_t expireTime = HAL_GetTick() + CAPTURE_TIMEOUT;
    uint8_t success = 0;
    while (HAL_GetTick() < expireTime)
    {
      uint32_t cc1Flag = __HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_CC1);
      uint32_t cc2Flag = __HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_CC2);

      if (cc1Flag && cc2Flag)
      {
        // 捕获到后立即清除标志位，避免误判
        __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC1);
        __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC2);
        success = 1;
        break;
      }
    }

    // 停止捕获
    HAL_TIM_IC_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_IC_Stop(&htim1, TIM_CHANNEL_2);

    // 计算并显示距离
    if (success == 1)
    {
      // 关键修正：用HAL_TIM_ReadCapturedValue读取捕获值（不是比较值）
      uint16_t ccr1 = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1);
      uint16_t ccr2 = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_2);

      // 处理计数器溢出（ccr2 < ccr1时补满量程）
      uint32_t pulse_cnt = (ccr2 >= ccr1) ? (ccr2 - ccr1) : (0xFFFF - ccr1 + ccr2);
      float pulsewidth = pulse_cnt * 1e-6;  // 脉冲宽度（秒）
      float distance_m = pulsewidth * 340 / 2.0;  // 距离（米）
      float distance_cm = distance_m * 100;      // 转换为厘米（适配OLED显示）

      // OLED显示厘米值（保留1位小数，四舍五入）
      OLED_ShowNum(1, 1, (uint32_t)(distance_cm + 0.5), 5);
    }
    else
    {
      OLED_ShowString(1, 1, "ERROR");  // 捕获失败提示
    }

    HAL_Delay(20);  // 测距间隔，避免频繁触发模块
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

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */