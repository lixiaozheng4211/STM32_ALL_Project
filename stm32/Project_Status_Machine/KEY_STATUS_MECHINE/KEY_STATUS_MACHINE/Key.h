#ifndef KEY_H
#define KEY_H

#include "tim.h"
#include "main.h"
/*
double click:
```___________``````````````___________````````````
     min< <max    <judge      min< <max     >judge

single click:
``````___________`````````
       min< <max  >judge

*/

#define LONG_PRESS_TIME 	     100
#define CLICK_MIN_TIME 		2	/* if key press_cnt time less than this -> invalid click */
#define CLICK_MAX_TIME 		30	/* if key press_cnt time more than this -> invalid click */
#define JUDGE_TIME 			20	/* double click time space */


typedef enum {
  KEY_NULL,
  KEY_DOWN,
  KEY_PRESS,
  KEY_UP,
} KeyActionType;

typedef enum {
  BUTTON_NULL,
  BUTTON_SINGLE,
  BUTTON_DOUBLE,
  BUTTON_TRIPLE,
  BUTTON_LONG_PRESS,
} ButtonActionType;

typedef struct {
  GPIO_TypeDef *GPIO_Port; //按键端口
  uint16_t GPIO_Pin; //按键PIN
  KeyActionType key; //按键类型
  uint16_t hold_cnt; //按压计数器
  uint16_t high_cnt; //高电平计数器
  uint8_t press_flag; //按压标志
  uint8_t release_flag; //松手标志
  ButtonActionType buttonAction; //按键键值
} buttonType;

extern buttonType button[2];

void Key_Scan(buttonType *);

void Key_Debug();

void Key_Config();

#endif
