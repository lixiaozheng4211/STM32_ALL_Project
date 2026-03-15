#ifndef FREERTOSQUEUE_KEY_STATUS_MACHINE_H
#define FREERTOSQUEUE_KEY_STATUS_MACHINE_H
#ifndef KEY_STATUS_MACHINE_TEST_KEY_H
#define KEY_STATUS_MACHINE_TEST_KEY_H
#include "stm32f1xx_hal.h"
#include "main.h"

#define BUTTON_SIZE 2
// 定义长按时间 , 单击 , 区间 , 判定
#define LONG_PRESS_EVENT 100
#define PRESS_MIN_TIME 10
#define PRESS_MAX_TIME 25
#define MULTI_JUDGE_TIME 20

// 动作定义
typedef enum {
  KeyNull = 0,
  KeyDown,
  KeyPressed,
  KeyUp
} KeyStatus;

// 事件定义
typedef enum {
  KEY_NULL_EVENT = 0,
  KEY_PRESS_SINGLE,
  KEY_PRESS_DOUBLE,
  KEY_PRESS_TRIPLE,
  KEY_LONG_PRESS,
} ButtonEvent;

// 定义状态机节点
typedef struct {
  GPIO_TypeDef *GPIOx;
  uint16_t GPIO_Pin;
  uint16_t High_Cnt;
  uint16_t Pressed_Cnt;
  uint8_t Pressed_Flag;
  uint8_t Release_Flag;
  KeyStatus KEY_Status;
  ButtonEvent ButtonEvent;
}Button_t;


void KEY_Para_Reset(Button_t*);

void Key_Scan(Button_t*);

void Key_Debug(void);

void Key_Init(void);

extern Button_t button_t[BUTTON_SIZE];

#endif //KEY_STATUS_MACHINE_TEST_KEY_H

#endif //FREERTOSQUEUE_KEY_STATUS_MACHINE_H