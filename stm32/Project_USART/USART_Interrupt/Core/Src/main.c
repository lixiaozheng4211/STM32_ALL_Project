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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
/* USER CODE END Includes */
#define False 0
#define True  1
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
uint8_t NonBlocking_DelayCheck(uint32_t start_time, uint32_t delay_ms) {
  // 处理HAL_GetTick()溢出（比如从0xFFFFFFFF回到0的情况）
  if (HAL_GetTick() - start_time >= delay_ms) {
    return 1; // 延迟时间已到
  }
  return 0; // 延迟未到
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 全局变量定义
uint8_t receiveDate[2] = {0};
volatile uint8_t uart1_received = 0;  // 接收完成标志，volatile确保多线程安全
volatile uint8_t uart1_need_response = 0;  // 需要回传标志
uint32_t display_start_time = 0;
uint8_t need_clear_message_date = 0;
uint8_t need_clear_message_IT = 0;
char display_msg[20] = {0};  // 存储显示消息


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    uart1_received =         1;
    uart1_need_response =    1;
    HAL_UART_Receive_IT(&huart1, receiveDate, sizeof(receiveDate));
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    OLED_ShowString(3, 1, "Tx_Is_OK");
  }
}

/**
  * @brief  应用入口函数
  * @retval int
  */
int main(void) {
  /* MCU初始化 */
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  /* 外设初始化 */
  OLED_Init();
  OLED_ShowString(1, 1, "USART Receive...");

  HAL_UART_Receive_IT(&huart1, receiveDate, sizeof(receiveDate));

  while (1) {
    // 1. 检查是否接收到新数据
    if (uart1_received) {
      uart1_received = 0;  // 清除标志

      // 显示接收到的数据
      OLED_ShowString(2, 1, display_msg);

      // 设置显示成功消息
      OLED_ShowString(2, 1, "ReceiveDateOK");
      display_start_time = HAL_GetTick();
      need_clear_message_date = 1;
      need_clear_message_IT = 1;

      // 根据接收到的数据控制GPIO
      if (receiveDate[0] == 'R') {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4,
                         (receiveDate[1] == '0') ? GPIO_PIN_SET : GPIO_PIN_RESET);
      } else if (receiveDate[0] == 'G') {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2,
                         (receiveDate[1] == '0') ? GPIO_PIN_SET : GPIO_PIN_RESET);
      } else if (receiveDate[0] == 'B') {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3,
                         (receiveDate[1] == '0') ? GPIO_PIN_SET : GPIO_PIN_RESET);
      }
    }

    // 2. 检查是否需要回传数据（使用中断发送）
    if (uart1_need_response) {
      uart1_need_response = False;
      // 检查串口状态是否就绪
      if (huart1.gState == HAL_UART_STATE_READY) {
        // 使用中断方式回传数据（非阻塞）
        HAL_UART_Transmit_IT(&huart1, receiveDate, sizeof(receiveDate));
      }
    }

    // 3. 检查是否该清除"ReceiveDateOK"消息
    if (need_clear_message_date && NonBlocking_DelayCheck(display_start_time, 1000)) {
      OLED_ShowString(2, 1, "                ");
      need_clear_message_date = 0;
    }

    // 4. 检查是否要清除"Tx_Is_OK"消息
    if (need_clear_message_IT && NonBlocking_DelayCheck(display_start_time , 500)) {
      OLED_ShowString(3, 1, "                ");
      need_clear_message_IT = 0;
    }
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
