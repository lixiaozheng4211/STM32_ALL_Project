// Constants and timing definitions for KEY_STATUS_MACHINE

#ifndef KEY_H
#define KEY_H

// Define the button count for the system
#define BUTTON_COUNT 4  // Number of buttons in the system

// Define time constants (in milliseconds) for button debounce and response
#define DEBOUNCE_TIME  20   // Time (in ms) to debounce button press (2 scans)
#define LONG_PRESS_TIME 300  // Time (in ms) to define a long press (30 scans)
#define DOUBLE_PRESS_TIME 200 // Time (in ms) to define a double press (20 scans)

// Detailed documentation on time thresholds:
// DEBOUNCE_TIME: This is the time required to stabilize the button reading after a press. 
//               It is defined as thresholds for 2 scans at 10ms period, hence 20ms total.
// LONG_PRESS_TIME: This value determines the duration for which the button must be held down
//                  for it to be recognized as a long press. It is measured at 30 scans, translating to 300ms.
// DOUBLE_PRESS_TIME: This threshold represents the maximum interval allowed between two
//                    button presses for them to be considered a double press. This is set to 200ms,
//                    indicating a time span of 20 scans.

<<<<<<< HEAD
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
=======
#endif // KEY_H
>>>>>>> 7ea6e9003e6d6f3832ab6b80aa4649ac657825cc
