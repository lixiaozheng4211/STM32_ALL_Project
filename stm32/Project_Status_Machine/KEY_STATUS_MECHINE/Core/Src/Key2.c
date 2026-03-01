#include "Key.h"

buttonType button2[2];

void Key_Debug2() {
  // 这个等下在复制一样的
}

void Key_Config2() {
  button->GPIO_Port = KEY1_GPIO_Port;
  button->GPIO_Pin = KEY1_Pin;

  button->GPIO_Port = KEY2_GPIO_Port;
  button->GPIO_Pin = KEY2_Pin;
}

void Key_ParaInit2(buttonType *button) {
  button->high_cnt = 0;
  button->hold_cnt = 0;
  button->press_flag = 0;
  button->release_flag = 0;
}

void Key_Scan2(buttonType *buttonKey) {
  switch (buttonKey->keyAction == KEY_NULL) {
    case KEY_NULL:
      // 有可能是没有按 , 也有可能是按下再松开
      if (HAL_GPIO_ReadPin(buttonKey->GPIO_Port, buttonKey->GPIO_Pin) == 0) {
        buttonKey->keyAction = KEY_DOWN;
      } else if (HAL_GPIO_ReadPin(buttonKey->GPIO_Port, buttonKey->GPIO_Pin) == 1) {
        buttonKey->keyAction = KEY_NULL;
      }

      if (buttonKey->release_flag == 1) {
        buttonKey->high_cnt++;
      }

      if (buttonKey->hold_cnt > LONG_PRESS_TIME) {
        buttonKey->buttonAction = BUTTON_LONG_PRESS;
        Key_ParaInit2(buttonKey);
      }
      if (HAL_GPIO_ReadPin(buttonKey->GPIO_Port, buttonKey->GPIO_Pin) == 0) {
        buttonKey->hold_cnt++;
        Key_ParaInit2(buttonKey);
      } else if (buttonKey->hold_cnt > CLICK_MAX_TIME && buttonKey->hold_cnt < LONG_PRESS_TIME) {
        Key_ParaInit2(buttonKey); // 表示这是一次误触 , 无事发生
      } else if (buttonKey->high_cnt > JUDGE_TIME && buttonKey->hold_cnt > CLICK_MIN_TIME && buttonKey->hold_cnt <
                 CLICK_MAX_TIME) {
        if (buttonKey->release_flag == 1) {
          buttonKey->buttonAction = BUTTON_SINGLE;
        } else if (buttonKey->release_flag == 2) {
          buttonKey->buttonAction = BUTTON_DOUBLE;
        } else if (buttonKey->release_flag == 3) {
          buttonKey->buttonAction = BUTTON_TRIPLE;
        }
        Key_ParaInit2(buttonKey);
      }
      break;
    case KEY_DOWN:
      buttonKey->keyAction = KEY_PRESS;
      buttonKey->press_flag++;
      buttonKey->release_flag = 0;
      buttonKey->hold_cnt = 0;
      break;
    case KEY_PRESS:
      if (HAL_GPIO_ReadPin(buttonKey->GPIO_Port , buttonKey->GPIO_Pin) == 0) {
        buttonKey->keyAction = KEY_PRESS;
        buttonKey->hold_cnt++;
      } else if (HAL_GPIO_ReadPin(buttonKey->GPIO_Port , buttonKey->GPIO_Pin) == 1) {
        buttonKey->keyAction = KEY_UP;
      }
      break;
    case KEY_UP:
      buttonKey->keyAction = KEY_NULL;
      buttonKey->high_cnt++;
      buttonKey->release_flag = 1;
      if (buttonKey->hold_cnt > 100) {
        button->press_flag--;
      }
      break;
    default:
      break;
  }
}
