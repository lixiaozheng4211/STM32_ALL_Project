#include "Key_Status_Machine.h"


Button_t button_t[BUTTON_SIZE];

void Key_Init(void) {
  button_t[0].GPIOx = KEY1_GPIO_Port;
  button_t[1].GPIOx = KEY2_GPIO_Port;
  button_t[0].GPIO_Pin = KEY1_Pin;
  button_t[1].GPIO_Pin = KEY2_Pin;
}

// 这就是按键重置函数
void KEY_Para_Reset(Button_t *Button) {
  Button->High_Cnt = 0;
  Button->Pressed_Cnt = 0;
  Button->Pressed_Flag = 0;
  Button->Release_Flag = 0;
}

void Key_Scan(Button_t *Button) {
  switch (Button->KEY_Status) {
    case KeyNull:
      if (HAL_GPIO_ReadPin(Button->GPIOx, Button->GPIO_Pin) == GPIO_PIN_RESET) {
        Button->KEY_Status = KeyDown;
      } else if (HAL_GPIO_ReadPin(Button->GPIOx, Button->GPIO_Pin) == GPIO_PIN_SET) {
        Button->KEY_Status = KeyNull;
      }

      if (Button->Release_Flag == 1) {
        Button->High_Cnt++;
      }

      // ===================== judge ===================
      if (Button->Pressed_Cnt < LONG_PRESS_EVENT && Button->Pressed_Cnt > PRESS_MAX_TIME) {
        KEY_Para_Reset(Button);
        // 如果超过多次连击的判定时间 , 就可以认为用户不会再按了 , 所以就可以结算当前的状态
      } else if (Button->High_Cnt > MULTI_JUDGE_TIME && Button->Pressed_Cnt > PRESS_MIN_TIME && Button->Pressed_Cnt <
                 PRESS_MAX_TIME) {
        if (Button->Pressed_Flag == 1) {
          Button->ButtonEvent = KEY_PRESS_SINGLE;
        } else if (Button->Pressed_Flag == 2) {
          Button->ButtonEvent = KEY_PRESS_DOUBLE;
        } else if (Button->Pressed_Flag == 3) {
          Button->ButtonEvent = KEY_PRESS_TRIPLE;
        }
        KEY_Para_Reset(Button);
      }
      break;
    case KeyDown:
      Button->KEY_Status = KeyPressed;
      Button->Pressed_Flag++;
      Button->Release_Flag = 0;
      Button->Pressed_Cnt = 0;
      break;
    case KeyPressed:
      if (Button->Pressed_Cnt > LONG_PRESS_EVENT) {
        Button->ButtonEvent = KEY_LONG_PRESS;
        KEY_Para_Reset(Button);
      }

      if (HAL_GPIO_ReadPin(Button->GPIOx, Button->GPIO_Pin) == GPIO_PIN_SET) {
        Button->KEY_Status = KeyUp;
      }

      if (HAL_GPIO_ReadPin(Button->GPIOx, Button->GPIO_Pin) == GPIO_PIN_RESET) {
        Button->KEY_Status = KeyPressed; // 改动 : 保留当前状态
        Button->Pressed_Cnt++;
      }
      break;
    case KeyUp:
      Button->Release_Flag = 1;
      Button->KEY_Status = KeyNull;
      Button->High_Cnt = 0;

      break;
    default:
      break;
  }
}
