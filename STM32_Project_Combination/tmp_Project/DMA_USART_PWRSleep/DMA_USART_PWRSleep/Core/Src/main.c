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
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>

#include "OLED.h"
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
#define RX_BUF_SIZE 128
uint8_t rx_buffer[RX_BUF_SIZE];
uint8_t process_buffer[RX_BUF_SIZE]; // 新增：用来存放“捋直”后的数据，方便OLED显示

volatile uint8_t data_ready_flag = 0;
volatile uint16_t current_rx_len = 0; // 本次实际收到的长度
uint16_t old_pos = 0; // 记录上次 DMA 写到的位置
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
void Delay(uint32_t nTime) {
  for (volatile uint8_t i = 0; i < 1 * nTime; i++) {
    __NOP();
  }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
  if (huart->Instance == USART1) {
    // 这是惯例
    uint16_t new_pos = Size; // Size 就是当前 DMA 的新写入位置

    // 没有发生回绕，数据是连续的
    if (new_pos > old_pos) {
      current_rx_len = new_pos - old_pos;
      // 直接把这一段连续的数据拷贝到处理区
      memcpy(process_buffer, &rx_buffer[old_pos], current_rx_len);
    }
    // 发生了回绕，数据被截断成了两段
    else if (new_pos < old_pos) {
      uint16_t part1_len = RX_BUF_SIZE - old_pos; // 尾部的一段
      uint16_t part2_len = new_pos; // 头部的一段
      current_rx_len = part1_len + part2_len;

      // 分两次拷贝，把两段数据拼接到一起
      memcpy(&process_buffer[0], &rx_buffer[old_pos], part1_len);
      memcpy(&process_buffer[part1_len], rx_buffer, part2_len);
    }
    // 更新历史位置，为下一次接收做准备
    old_pos = new_pos;
    // 告诉 main 函数有新数据了
    data_ready_flag = 1;
    // 回显数据：直接发送我们刚刚提取好的 process_buffer
    HAL_UART_Transmit_DMA(huart, process_buffer, current_rx_len);
  }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  OLED_Init();

  // 开启串口空闲中断 + DMA 接收
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer, RX_BUF_SIZE);

  // 移到循环外面，只在最开始和处理完数据后打印，防止被 SysTick 唤醒后狂刷
  OLED_ShowString(2, 1, "Waiting for data");

  while (1) {
    if (data_ready_flag == 1) {
      // 1. 清理一下标志位
      data_ready_flag = 0;

      // 2. 清除 OLED 上的上一条信息 (你自己写好的那些提示)
      OLED_ShowString(1, 1, "                ");

      // 3. 打印最新收到的数据 (直接用 current_rx_len 和 process_buffer)
      for (uint8_t i = 0; i < current_rx_len; i++) {
        // 注意：如果收到的数据超出了 OLED 一行的显示范围，这里要做换行处理，
        // 为了简单，我们先假设它不会超过 16 个字符
        if (i < 16) {
          OLED_ShowChar(1, i + 1, process_buffer[i]);
        }
      }

      HAL_Delay(500); // 延时一下方便观察
    }

    // 继续睡觉，等待下一次空闲中断唤醒
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
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
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
