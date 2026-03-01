#include "key.h"
#include "stdio.h"

buttonType button[2];

/*    注意 : 这里的KEY_DOWN还有KEY_UP都是瞬间状态 , 只是为了捕捉高低电平的变化
 *    Current_S     Target_S      Action
 *    KEY_IDLE      KEY_DOWN      readPin == 0
 *    KEY_DOWN      KEY_PRESSED   None
 *    KEY_PRESSED   KEY_PRESSED   while(readPin == 0)
 *    KEY_PRESSED   KEY_UP        readPin == 1
 *    KEY_UP        KEY_IDLE      Done
 *
 * 时间轴：0ms       20ms(CLICK_MIN)    300ms(CLICK_MAX)    1000ms(LONG_PRESS)
 *       |------------|-------------------|--------------------|--------------
 *            抖动区        有效单击区          无效区               长按区
 *              ↓            ↓                  ↓                   ↓
 *         <---忽略---> <--判定为短按---> <--既不是短按也不是长按--> <--判定为长按-->
 */


/* 单击识别
时间轴：0ms   50ms   250ms ======== 450ms
动作：  [按下]=======[释放] [无操作]
       │            │              │
       │ 按压200ms   │ 释放后200ms  │
       │ hold_cnt=20│ high_cnt=20  │
       └────────────┴──────────────┘

1. 按压200ms：20>5且20<30 ✓（有效按压时间）
2. 释放200ms：20>20？✗（200ms=200ms，不满足>200ms）
3. 释放250ms：25>20 ✓（超过判定时间）
4. 此时满足条件：输出BUTTON_SINGLE
*/


/* 双击识别
// 假设扫描周期为10ms，JUDGE_TIME=20（即200ms）

第一次点击：按下→释放 → high_cnt开始计数
    ↓
如果用户在200ms内再次按下：high_cnt < 20
    ↓
这是"双击"的一部分，继续等待
    ↓
如果200ms后用户没有再次按下：high_cnt > 20
    ↓
认为"连续点击动作已结束"，可以输出结果
*/

void Key_Config() {
  button[0].GPIO_Port = KEY1_GPIO_Port;
  button[0].GPIO_Pin = KEY1_Pin;

  button[1].GPIO_Port = KEY2_GPIO_Port;
  button[1].GPIO_Pin = KEY2_Pin;

}

void Key_ParaInit(buttonType *button) {
  button->high_cnt = 0;
  button->hold_cnt = 0;
  button->press_flag = 0;
  button->release_flag = 0;
}

void Key_Scan(buttonType *button) {
  switch (button->key) {
    case KEY_NULL: {
      /* if falling edge captured */
      if (HAL_GPIO_ReadPin(button->GPIO_Port, button->GPIO_Pin) == 0) {
        button->key = KEY_DOWN;
      } else if (HAL_GPIO_ReadPin(button->GPIO_Port, button->GPIO_Pin) == 1) {
        button->key = KEY_NULL;
      }

      /* if button is released ,high_time_count++ */
      if (button->release_flag == 1) {
        button->high_cnt++;
      }

      /**********************judge***********************/
      /* if high_time_count is longer than LONG_PRESS_TIME, consider BUTTON_LONG_PRESS */
      if (button->hold_cnt > LONG_PRESS_TIME) {
        button->buttonAction = BUTTON_LONG_PRESS;
        Key_ParaInit(button);
      }
      /* if high_time_count is shorter than LONG_PRESS_TIME,but longer than CLICK_MAX_TIME consider INVALID */
      else if (button->hold_cnt < LONG_PRESS_TIME && button->hold_cnt > CLICK_MAX_TIME) {
        Key_ParaInit(button);
      }

      /*
        only the latest press time is in range of [CLICK_MIN_TIME,CLICK_MAX_TIME] can be regarded valid
        if high level time > JUDGE_TIME also means that over the JUDGE_TIME and still dont have button pushed
        we can check the flag value to get button state now
      */
      else if ((button->high_cnt > JUDGE_TIME) && (
                 button->hold_cnt > CLICK_MIN_TIME && button->hold_cnt < CLICK_MAX_TIME)) {
        if (button->press_flag == 1) {
          button->buttonAction = BUTTON_SINGLE;
        } else if (button->press_flag == 2) {
          button->buttonAction = BUTTON_DOUBLE;
        } else if (button->press_flag == 3) {
          button->buttonAction = BUTTON_TRIPLE;
        }

        Key_ParaInit(button);
      }
      break;
    }

    case KEY_DOWN: {
      button->key = KEY_PRESS;

      /* as long as falling edge occurring,press_flag++ */
      button->press_flag++;

      button->release_flag = 0; /* means that the button has been pressed */

      button->hold_cnt = 0; /* reset hold time count */
      break;
    }

    case KEY_PRESS: {
      /* when button was kept pressed, hold count++ */
      if (HAL_GPIO_ReadPin(button->GPIO_Port, button->GPIO_Pin) == 0) {
        button->key = KEY_PRESS;
        button->hold_cnt++;
      }
      /* when button was released, change state */
      else if (HAL_GPIO_ReadPin(button->GPIO_Port, button->GPIO_Pin) == 1) {
        button->key = KEY_UP;
      }
      break;
    }

    case KEY_UP: {
      button->key = KEY_NULL;

      button->release_flag = 1; /* means that the button is released */

      button->high_cnt = 0; /* reset hold time count */

      /* if press time is longer than 1s then press_flag-- */
      if (button->hold_cnt > 100) {
        button->press_flag--;
      }
      break;
    }
    default:
      break;
  }
}


void Key_Debug() {
  for (uint8_t i = 0; i < 2; i++) {
    switch (button[i].buttonAction) {
      case BUTTON_SINGLE: {
        button[i].buttonAction = BUTTON_NULL;
        printf("%d->", i);
        printf("BUTTON_SINGLE\r\n");
        break;
      }
      case BUTTON_LONG_PRESS: {
        button[i].buttonAction = BUTTON_NULL;
        printf("%d->", i);
        printf("BUTTON_LONG_PRESS\r\n");
        break;
      }
      case BUTTON_DOUBLE: {
        button[i].buttonAction = BUTTON_NULL;
        printf("%d->", i);
        printf("BUTTON_DOUBLE\r\n");
        break;
      }
      case BUTTON_TRIPLE: {
        button[i].buttonAction = BUTTON_NULL;
        printf("%d->", i);
        printf("BUTTON_TRIPLE\r\n");
        break;
      }
      case BUTTON_NULL: {
        button[i].buttonAction = BUTTON_NULL;
        break;
      }
      default: {
        break;
      }
    }
  }
}
