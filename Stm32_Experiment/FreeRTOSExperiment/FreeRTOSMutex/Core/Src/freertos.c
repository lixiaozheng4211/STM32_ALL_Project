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
#include "LED.h"
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
osThreadId EchoTaskHandle;
osMutexId myMutex01Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const *argument);

void myTaskLED1(void const *argument);

void myTaskLED2(void const *argument);

void myTaskEcho(void const *argument);

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
  /* definition and creation of myMutex01 */
  osMutexDef(myMutex01);
  myMutex01Handle = osMutexCreate(osMutex(myMutex01));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of LED1 */
  osThreadDef(LED1, myTaskLED1, osPriorityNormal, 0, 128);
  LED1Handle = osThreadCreate(osThread(LED1), NULL);

  /* definition and creation of LED2 */
  osThreadDef(LED2, myTaskLED2, osPriorityNormal, 0, 128);
  LED2Handle = osThreadCreate(osThread(LED2), NULL);

  /* definition and creation of Echo */
  osThreadDef(Echo, myTaskEcho, osPriorityAboveNormal, 0, 256);
  EchoTaskHandle = osThreadCreate(osThread(Echo), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
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

/* USER CODE BEGIN Header_myTaskLED1 */
/**
* @brief Function implementing the LED1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_myTaskLED1 */
void myTaskLED1(void const *argument) {
  /* USER CODE BEGIN myTaskLED1 */
  uint8_t blink_count = 0;

  for (;;) {
    LED_Control(LED1_GPIO_Port, GPIO_PIN_0, LED_ON);
    osDelay(600);
    LED_Control(LED1_GPIO_Port, GPIO_PIN_0, LED_OFF);
    osDelay(600);
    LED_Control(LED1_GPIO_Port, GPIO_PIN_0, LED_TOGGLE);
    osDelay(600);

    blink_count++;

    /* 每闪烁5次发送一条串口消息，两个任务同时触发时互斥锁会让其中一个等待 */
    if (blink_count >= 5) {
      blink_count = 0;
      UART_SendWithMutex("LED1: blinked 5 times\r\n");
    }
  }
  /* USER CODE END myTaskLED1 */
}

/* USER CODE BEGIN Header_myTaskLED2 */
/**
* @brief Function implementing the LED2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_myTaskLED2 */
void myTaskLED2(void const *argument) {
  /* USER CODE BEGIN myTaskLED2 */
  uint8_t blink_count = 0;

  for (;;) {
    LED_Control(LED2_GPIO_Port, GPIO_PIN_1, LED_ON);
    osDelay(200);
    LED_Control(LED2_GPIO_Port, GPIO_PIN_1, LED_OFF);
    osDelay(200);
    LED_Control(LED2_GPIO_Port, GPIO_PIN_1, LED_TOGGLE);
    osDelay(200);

    blink_count++;

    /* LED2 周期更短，会更频繁触发，和 LED1 抢互斥锁的概率更高 */
    if (blink_count >= 5) {
      blink_count = 0;
      UART_SendWithMutex("LED2: blinked 5 times\r\n");
    }
  }
  /* USER CODE END myTaskLED2 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
 * @brief Echo任务：等待接收通知，收到后原样回发数据
 *
 * 启动时先开启DMA接收，然后阻塞等待Task Notification。
 * 中断回调触发后，用互斥锁保护串口，把收到的数据发回去，
 * 再重新开启下一次接收，形成循环。
 */
void myTaskEcho(void const *argument) {
  /* 开启第一次接收 */
  UART_StartReceive();

  for (;;) {
    /* 无限等待通知，通知值即为接收到的字节数 */
    uint32_t rx_len = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (rx_len > 0 && rx_len <= UART_RX_BUF_SIZE) {
      /* 用互斥锁保护发送，和LED任务共享同一把锁 */
      if (osMutexWait(myMutex01Handle, 1000) == osOK) {
        HAL_UART_Transmit_DMA(&huart1, uart_rx_buf, (uint16_t) rx_len);

        while (HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX ||
               HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX_RX) {
          osDelay(1);
        }

        osMutexRelease(myMutex01Handle);
      }
    }

    /* 重新开启下一次接收 */
    UART_StartReceive();
  }
}

/* USER CODE END Application */
