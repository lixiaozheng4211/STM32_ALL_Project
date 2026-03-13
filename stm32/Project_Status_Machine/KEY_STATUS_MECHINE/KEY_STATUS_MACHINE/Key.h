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
#define CLICK_MAX_TIME 		15	/* if key press_cnt time more than this -> invalid click */
#define JUDGE_TIME 			10	/* double click time space */


typedef enum {
  KEY_NULL, // 按键空闲状态
  KEY_DOWN, // 按键按下的一瞬间
  KEY_PRESS, // 按键一直按下
  KEY_UP, // 按键抬起的一瞬间
} KeyActionType;

typedef enum {
  BUTTON_NULL, // 按键没有动作
  BUTTON_SINGLE, // 单击
  BUTTON_DOUBLE, // 双击
  BUTTON_TRIPLE, // 三击
  BUTTON_LONG_PRESS, // 长按
} ButtonActionType;

typedef struct {
  GPIO_TypeDef *GPIO_Port; //按键端口
  uint16_t GPIO_Pin; //按键PIN
  KeyActionType keyAction; //按键类型
  uint16_t hold_cnt; //按压计数器 (目的是记录按压时间)
  uint16_t high_cnt; //高电平计数器 (目的是区别双击和两次单击)
  uint8_t press_flag;
  //按压标志 每次进 KEY_DOWN，它就会 ++。如果在 JUDGE_TIME 规定的时间内按了两次，它就会变成 2；最后状态机就是靠判断 press_flag == 2 来输出 BUTTON_DOUBLE（双击）的。
  uint8_t release_flag; //松手标志 (KEY_UP时候使用)
  ButtonActionType buttonAction; //按键键值 (按键动作 , 启动定时器)
} buttonType;

extern buttonType button[2];

void Key_Scan(buttonType *);

void Key_Debug();

void Key_Config();

void Key_ParaInit(buttonType *button);
#endif
