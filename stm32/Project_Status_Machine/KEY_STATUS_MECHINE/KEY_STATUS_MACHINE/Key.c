#include "Key.h"
#include "usart.h"

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

void Key_Scan(buttonType *button) {
  // 1. 先更新计数器
  if (button->keyStatus == KEY_PRESSED) {
    // 按键按住期间，递增按压时间
    button->hold_cnt++;

    // 在按住期间判断长按
    if (button->hold_cnt > LONG_PRESS_TIME) {
      button->buttonAction = BUTTON_LONG_PRESS;
      Key_ParaInit(button);
      button->keyStatus = KEY_IDLE; // 重置状态
      return; // 立即返回，不再执行后续状态转移
    }
  } else if (button->keyStatus == KEY_IDLE && button->release_flag == 1) {
    // 按键释放后，递增释放时间
    button->high_cnt++;
  }

  // 2. 状态机处理
  switch (button->keyStatus) {
    case KEY_IDLE: {
      // 检测下降沿（按键按下）
      if (HAL_GPIO_ReadPin(button->GPIO_Port, button->GPIO_Pin) == 0) {
        button->keyStatus = KEY_DOWN;
      } else {
        // 按键未按下，检查是否可以判定短按动作
        if (button->release_flag == 1 && button->high_cnt > JUDGE_TIME) {
          // 距离上次释放已超过判定时间，可以输出结果
          if (button->hold_cnt > CLICK_MIN_TIME &&
              button->hold_cnt < CLICK_MAX_TIME) {
            // 有效短按，根据连续按压次数判断
            switch (button->press_flag) {
              case 1:
                button->buttonAction = BUTTON_SINGLE;
                break;
              case 2:
                button->buttonAction = BUTTON_DOUBLE;
                break;
              case 3:
                button->buttonAction = BUTTON_TRIPLE; // 修正为三击
                break;
              default:
                break;
            }
          } else if (button->hold_cnt >= CLICK_MAX_TIME &&
                     button->hold_cnt < LONG_PRESS_TIME) {
            // 按压时间在无效区，忽略
            // 不输出任何动作
          }

          // 判定完成后初始化参数
          Key_ParaInit(button);
        }
      }
      break;
    }

    case KEY_DOWN: {
      // 确认按键按下（消抖）
      if (HAL_GPIO_ReadPin(button->GPIO_Port, button->GPIO_Pin) == 0) {
        button->keyStatus = KEY_PRESSED;
        button->press_flag++; // 只在按下瞬间递增一次
        button->release_flag = 0; // 标记为按下状态
        button->hold_cnt = 0; // 重置按压时间计数器
      } else {
        // 可能是抖动，回到空闲状态
        button->keyStatus = KEY_IDLE;
      }
      break;
    }

    case KEY_PRESSED: {
      // 长按已在函数开头判断
      // 检测上升沿（按键释放）
      if (HAL_GPIO_ReadPin(button->GPIO_Port, button->GPIO_Pin) == 1) {
        button->keyStatus = KEY_UP;
      }
      // 保持按住状态，hold_cnt已在开头递增
      break;
    }

    case KEY_UP: {
      button->keyStatus = KEY_IDLE;
      button->release_flag = 1; // 标记为释放状态
      button->high_cnt = 0; // 重置释放时间计数器

      // 如果是长按，不计入连续按压次数
      if (button->hold_cnt > LONG_PRESS_TIME) {
        button->press_flag--; // 长按不计入连续按压
      }
      break;
    }

    default:
      button->keyStatus = KEY_IDLE;
      break;
  }
}

void Key_ParaInit(buttonType *button) {
  button->keyStatus = KEY_IDLE;
  button->high_cnt = 0;
  button->hold_cnt = 0;
  button->press_flag = 0;
  button->release_flag = 0;
}


void Key_Debug(void) {
  for (uint8_t i = 0; i < 2; i++) {
    switch (button[i].buttonAction) {
      case BUTTON_SINGLE: {
        printf("Button%d: SINGLE CLICK\n", i);
        button[i].buttonAction = BUTTON_NULL;
        break;
      }
      case BUTTON_DOUBLE: {
        printf("Button%d: DOUBLE CLICK\n", i);
        button[i].buttonAction = BUTTON_NULL;
        break;
      }
      case BUTTON_TRIPLE: {
        printf("Button%d: TRIPLE CLICK\n", i); // 修正输出
        button[i].buttonAction = BUTTON_NULL;
        break;
      }
      case BUTTON_LONG_PRESS: {
        printf("Button%d: LONG PRESS\n", i);
        button[i].buttonAction = BUTTON_NULL;
        break;
      }
      default:
        break;
    }
  }
}

void Key_Config(void) {
  for (int i = 0; i < 2; i++) {
    button[i].GPIO_Port = (i == 0) ? GPIOA : GPIOA;
    button[i].GPIO_Pin = (i == 0) ? GPIO_PIN_1 : GPIO_PIN_2;
    Key_ParaInit(&button[i]);
  }
}


void Key_Process(void) {
  for (int i = 0; i < 2; i++) {
    Key_Scan(&button[i]);
  }
  Key_Debug(); // 调试输出
}
