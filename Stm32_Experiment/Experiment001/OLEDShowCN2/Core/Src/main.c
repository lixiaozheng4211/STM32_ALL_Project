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
#include "OLED.h"
#include "UART_DMA.h"
#include "ESP8266.h"
#include "stdio.h"
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
UART_IO_t  uart_pc;    /* 串口1：与电脑通信 */
UART_IO_t  uart_wifi;  /* 串口2：与ESP8266通信 */
ESP8266_t  esp;        /* ESP8266 实例 */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ESP8266 收到服务器数据时的回调 */
static void on_wifi_data(const char *data, uint16_t len)
{
    /* 把收到的数据转发到电脑串口 */
    UART_IO_Send(&uart_pc, (const uint8_t *)data, len);
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
  OLED_Init();
  OLED_ShowCN(1,1,0);
  OLED_ShowString(2,1, "hello world");
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  UART_IO_Init(&uart_pc,   &huart1);
  UART_IO_Init(&uart_wifi, &huart2);
  UART_IO_StartReceive(&uart_pc);
  UART_IO_StartReceive(&uart_wifi);

  ESP8266_Init(&esp, &uart_wifi, on_wifi_data);

  UART_IO_Printf(&uart_pc, "Resetting ESP8266...\r\n");
  if (ESP8266_Reset(&esp) != ESP_OK) {
      UART_IO_Printf(&uart_pc, "Reset failed\r\n");
  }

  UART_IO_Printf(&uart_pc, "Connecting WiFi...\r\n");
  if (ESP8266_ConnectWiFi(&esp, "306", "1234m56m") != ESP_OK) {
      UART_IO_Printf(&uart_pc, "WiFi failed\r\n");
  }

  UART_IO_Printf(&uart_pc, "Connecting TCP...\r\n");
  if (ESP8266_ConnectTCP(&esp, "192.168.1.100", 8080) != ESP_OK) {
      UART_IO_Printf(&uart_pc, "TCP failed\r\n");
  } else {
      UART_IO_Printf(&uart_pc, "Connected!\r\n");
  }

  /* 获取 NTP 时间（东八区 UTC+8） */
  ESP_Time_t now;
  if (ESP8266_GetTime(&esp, 8, &now) == ESP_OK) {
      UART_IO_Printf(&uart_pc, "Time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
                     now.year, now.month, now.day,
                     now.hour, now.minute, now.second);
  } else {
      UART_IO_Printf(&uart_pc, "Get time failed\r\n");
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* 处理 ESP8266 收到的数据（触发 on_wifi_data 回调） */
    ESP8266_Process(&esp);

    /* 电脑发来的数据 -> 通过 ESP8266 发出去 */
    uint8_t ch;
    char    cmd_buf[128];
    uint8_t cmd_len = 0;
    while (UART_IO_ReadByte(&uart_pc, &ch)) {
        if (ch == '\n' && cmd_len > 0) {
            /* 收到换行，发送这一行 */
            ESP8266_Send(&esp, cmd_buf, cmd_len);
            cmd_len = 0;
        } else if (ch != '\r' && cmd_len < sizeof(cmd_buf) - 1) {
            cmd_buf[cmd_len++] = (char)ch;
        }
    }

    HAL_Delay(1);
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  UART_IO_TxCpltCallback(&uart_pc,   huart);
  UART_IO_TxCpltCallback(&uart_wifi, huart);
}
