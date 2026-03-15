/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LED_Control.h"
#include "Key_Status_Machine.h"
#include "UART_DMA.h"
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
/* USER CODE BEGIN Variables */
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId LED1Handle;
osThreadId LED2Handle;
osThreadId KeyScanHandle;
osThreadId EchoTaskHandle;
osMessageQId LED1QueueHandle;
osMessageQId LED2QueueHandle;
osMutexId UARTMutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const *argument);

void myTask01(void const *argument);

void myTask02(void const *argument);

void KeyScanfTask(void const *argument);

void EchoTask(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}

/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of UARTMutex */
  osMutexDef(UARTMutex);
  UARTMutexHandle = osMutexCreate(osMutex(UARTMutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of LED1Queue */
  osMessageQDef(LED1Queue, 16, uint16_t);
  LED1QueueHandle = osMessageCreate(osMessageQ(LED1Queue), NULL);

  /* definition and creation of LED2Queue */
  osMessageQDef(LED2Queue, 16, uint16_t);
  LED2QueueHandle = osMessageCreate(osMessageQ(LED2Queue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of LED1 */
  osThreadDef(LED1, myTask01, osPriorityNormal, 0, 128);
  LED1Handle = osThreadCreate(osThread(LED1), NULL);

  /* definition and creation of LED2 */
  osThreadDef(LED2, myTask02, osPriorityNormal, 0, 128);
  LED2Handle = osThreadCreate(osThread(LED2), NULL);

  /* definition and creation of KeyScan */
  osThreadDef(KeyScan, KeyScanfTask, osPriorityAboveNormal, 0, 128);
  KeyScanHandle = osThreadCreate(osThread(KeyScan), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* definition and creation of EchoTask */
  osThreadDef(Echo, EchoTask, osPriorityNormal, 0, 256);
  EchoTaskHandle = osThreadCreate(osThread(Echo), NULL);
  /* USER CODE END RTOS_THREADS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const *argument) {
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for (;;) {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_myTask01 */
/**
* @brief Function implementing the LED1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_myTask01 */
void myTask01(void const *argument) {
  /* USER CODE BEGIN myTask01 */
  osEvent evt;
  for (;;) {
    evt = osMessageGet(LED1QueueHandle, osWaitForever);
    if (evt.status == osEventMessage) {
      if (evt.value.v == 1) {
        // 单击：toggle 一次
        LED_Control(LED1_GPIO_Port, LED1_Pin, LED_TOGGLE);
        UART_SendWithMutex("[LED1] Single click, LED1 toggled\r\n");
      } else if (evt.value.v == 2) {
        // 双击：闪烁 3 次
        UART_SendWithMutex("[LED1] Double click, LED1 blink x3\r\n");
        for (int i = 0; i < 3; i++) {
          LED_Control(LED1_GPIO_Port, LED1_Pin, LED_TOGGLE);
          osDelay(150);
          LED_Control(LED1_GPIO_Port, LED1_Pin, LED_TOGGLE);
          osDelay(150);
        }
      }
    }
  }
  /* USER CODE END myTask01 */
}

/* USER CODE BEGIN Header_myTask02 */
/**
* @brief Function implementing the LED2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_myTask02 */
void myTask02(void const *argument) {
  /* USER CODE BEGIN myTask02 */
  osEvent evt;
  for (;;) {
    evt = osMessageGet(LED2QueueHandle, osWaitForever);
    if (evt.status == osEventMessage) {
      if (evt.value.v == 1) {
        // 单击：toggle 一次
        LED_Control(LED2_GPIO_Port, LED2_Pin, LED_TOGGLE);
        UART_SendWithMutex("[LED2] Single click, LED2 toggled\r\n");
      } else if (evt.value.v == 2) {
        // 双击：闪烁 3 次
        UART_SendWithMutex("[LED2] Double click, LED2 blink x3\r\n");
        for (int i = 0; i < 3; i++) {
          LED_Control(LED2_GPIO_Port, LED2_Pin, LED_TOGGLE);
          osDelay(150);
          LED_Control(LED2_GPIO_Port, LED2_Pin, LED_TOGGLE);
          osDelay(150);
        }
      }
    }
  }

/* USER CODE END myTask02 */
}

/* USER CODE BEGIN Header_KeyScanfTask */
/**
* @brief Function implementing the KeyScan thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KeyScanfTask */
void KeyScanfTask(void const *argument) {
  /* USER CODE BEGIN KeyScanfTask */
  Key_Init();
  /* Infinite loop */
  for (;;) {
    for (int i = 0; i < BUTTON_SIZE; i++) {
      Key_Scan(&button_t[i]);
    }

    if (button_t[0].ButtonEvent == KEY_PRESS_SINGLE) {
      button_t[0].ButtonEvent = KEY_NULL_EVENT;
      osMessagePut(LED1QueueHandle, 1, 0);
    } else if (button_t[0].ButtonEvent == KEY_PRESS_DOUBLE) {
      button_t[0].ButtonEvent = KEY_NULL_EVENT;
      osMessagePut(LED1QueueHandle, 2, 0);
    }

    if (button_t[1].ButtonEvent == KEY_PRESS_SINGLE) {
      button_t[1].ButtonEvent = KEY_NULL_EVENT;
      osMessagePut(LED2QueueHandle, 1, 0);
    } else if (button_t[1].ButtonEvent == KEY_PRESS_DOUBLE) {
      button_t[1].ButtonEvent = KEY_NULL_EVENT;
      osMessagePut(LED2QueueHandle, 2, 0);
    }

    osDelay(5);
  }
  /* USER CODE END KeyScanfTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void EchoTask(void const *argument) {
  uint32_t rxSize;
  UART_StartReceive(); // 开启第一次 DMA 接收

  for (;;) {
    // 阻塞等待中断回调发来的 Task Notification（值为接收到的字节数）
    if (xTaskNotifyWait(0, 0xFFFFFFFF, &rxSize, osWaitForever) == pdTRUE) {
      // 回显收到的数据
      if (osMutexWait(UARTMutexHandle, 1000) == osOK) {
        HAL_UART_Transmit_DMA(&huart1, uart_rx_buf, (uint16_t) rxSize);
        while (HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX ||
               HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX_RX) {
          osDelay(1);
        }
        osMutexRelease(UARTMutexHandle);
      }
      // 重新开启下一次接收
      UART_StartReceive();
    }
  }
}

/* USER CODE END Application */
