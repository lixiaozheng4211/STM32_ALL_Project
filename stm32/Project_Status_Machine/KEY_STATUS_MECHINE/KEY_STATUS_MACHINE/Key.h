#ifndef KEY_STATUS_MECHINE_KEY_H
#define KEY_STATUS_MECHINE_KEY_H
#include "stm32f1xx_hal.h"
#include "stdio.h"


// 现在定义单次按键的最短时间和最长时间
#define LONG_PRESS_TIME 	      100
#define CLICK_MIN_TIME 		      5	  /* if key press_cnt time less than this -> invalid click */
#define CLICK_MAX_TIME 		      30	/* if key press_cnt time more than this -> invalid click */
#define JUDGE_TIME 			        20	/* double click time space */

// 按键状态定义
typedef enum {
  KEY_IDLE = 0,
  KEY_DOWN,
  KEY_PRESSED,
  KEY_UP,
}KeyActionType;

// ButtonAction结构体定义
typedef enum {
  BUTTON_NULL,               // 空闲
  BUTTON_SINGLE,             // 单击
  BUTTON_DOUBLE,             // 双击
  BUTTON_TRIPLE,             // 三击
  BUTTON_LONG_PRESS,         // 长按
}ButtonActionType;

// 按键结构体
typedef struct
{
  GPIO_TypeDef*       GPIO_Port;		      //按键端口
  uint16_t            GPIO_Pin;				    //按键PIN
  KeyActionType       keyStatus;				  //按键类型
  uint16_t            hold_cnt;				    //按键保持时间计数器（低电平时间）区分长按vs短按
  uint16_t            high_cnt;				    //高电平时间计数器（释放后时间）判断连续按键的间隔
  uint8_t             press_flag;				  //按压标志
  uint8_t             release_flag;			  //松手标志
  ButtonActionType    buttonAction;	      //按键键值
}buttonType;


extern buttonType button[2];   // 对外声明现在有四个按键

void Key_Scan(buttonType*);    // 按键扫描
void Key_Debug();              // 这个其实可以不用 , 但是为了调试还是可以考虑
void Key_Config();             // 单纯为了配置
void Key_ParaInit(buttonType* button);       // 初始化 / 复位参数

#endif //KEY_STATUS_MECHINE_KEY_H