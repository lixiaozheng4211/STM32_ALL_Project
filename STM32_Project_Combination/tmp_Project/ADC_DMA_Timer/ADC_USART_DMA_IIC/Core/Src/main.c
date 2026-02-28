/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 最终优化版：ADC+DMA+TIM3触发+模拟看门狗+非阻塞延时
  * @note           : 硬件假设：蜂鸣器低电平触发（RESET=响，SET=关），请根据实际原理图修改！
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "stdio.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define OLED_REFRESH_MS 200    // OLED刷新间隔（非阻塞）
#define WATCHDOG_CLEAR_MS 3000 // 模拟看门狗自动清除报警的时间（3秒，无人值守场景用）
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
// ===================== 必须全局的变量（跨中断/主循环访问） =====================
uint16_t adc_buffer[2]; // ADC DMA双缓冲区：[0]=Rank1, [1]=Rank2(假设是被监控的通道9)
uint8_t WatchDogWarning = 0; // 模拟看门狗报警标志位
uint8_t uart_tx_done = 1; // 串口DMA发送完成标志位
uint32_t watchdog_trigger_tick = 0; // 记录模拟看门狗触发的时间（用于自动清除）
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  程序入口
  * @retval int
  */
int main(void) {
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  // ===================== 显式初始化关键外设/变量 =====================
  OLED_Init();
  OLED_Clear();
  uart_tx_done = 1;
  WatchDogWarning = 0;
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET); // 初始关闭蜂鸣器（硬件假设：SET=关）

  // ===================== 启动核心外设 =====================
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *) adc_buffer, 2); // 仅启动一次，循环模式自动刷新
  HAL_TIM_Base_Start(&htim3); // 仅启动TIM3硬件，产生TRGO触发ADC（不开启TIM3中断）

  // ===================== 非阻塞延时的时间戳变量 =====================
  uint32_t oled_refresh_tick = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    // ===================== 1. 非阻塞刷新OLED（避免HAL_Delay阻塞） =====================
    if (HAL_GetTick() - oled_refresh_tick >= OLED_REFRESH_MS) {
      oled_refresh_tick = HAL_GetTick();

      // 显示前补空格，清除残留（假设OLED一行最多显示4位数字）
      OLED_ShowString(1, 1, "    ");
      OLED_ShowNum(1, 1, adc_buffer[0], 4);
      OLED_ShowString(2, 1, "    ");
      OLED_ShowNum(2, 1, adc_buffer[1], 4);
    }

    // ===================== 2. 非阻塞串口发送（仅在上一次发送完成后执行） =====================
    if (uart_tx_done == 1) {
      // 非必要变量放在主循环内部
      uint8_t tx_buffer[64];
      float voltage1 = (float) adc_buffer[0] * 3.3f / 4095.0f;
      float voltage2 = (float) adc_buffer[1] * 3.3f / 4095.0f;

      snprintf((char *) tx_buffer, sizeof(tx_buffer),
               "S1:%4d(mV), S2:%4d(mV)\r\n",
               (int) (voltage1 * 1000), (int) (voltage2 * 1000));

      uart_tx_done = 0;
      HAL_UART_Transmit_DMA(&huart1, tx_buffer, strlen((char *) tx_buffer));
    }

    if (WatchDogWarning == 1) {
      // 触发报警
      HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET); // 硬件假设：RESET=响
      OLED_ShowString(3, 1, "Voltage Error!");

      // 自动清除报警（3秒后，无人值守场景用；也可改成按键清除）
      if (HAL_GetTick() - watchdog_trigger_tick >= WATCHDOG_CLEAR_MS) {
        WatchDogWarning = 0;
      }
    } else {
      // 恢复正常
      HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET); // 硬件假设：SET=关
      OLED_ShowString(3, 1, "              ");
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

// ===================== 1. ADC模拟看门狗越界回调（仅置位标志+记录时间，不做耗时操作） =====================
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc) {
  if (hadc->Instance == ADC1) {
    WatchDogWarning = 1;
    watchdog_trigger_tick = HAL_GetTick(); // 记录触发时间，用于自动清除
  }
}

// ===================== 2. 串口DMA发送完成回调 =====================
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    uart_tx_done = 1;
  }
}

/* USER CODE END 4 */

/**
  * @brief  错误处理函数
  * @retval None
  */
void Error_Handler(void) {
  __disable_irq();
  while (1) {
    OLED_ShowString(3, 1, "System Error!");
    HAL_Delay(500);
    OLED_ShowString(3, 1, "              ");
    HAL_Delay(500);
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
  OLED_ShowString(3, 1, "Assert Error");
}
#endif /* USE_FULL_ASSERT */
