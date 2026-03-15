#ifndef FREERTOSMUTEX_LED_H
#define FREERTOSMUTEX_LED_H
#include "main.h"

extern uint16_t LED1_Count , LED2_Count;

typedef enum {
  LED_ON = 0,
  LED_OFF = 1,
  LED_TOGGLE = 2,
}CmdStatus;

void LED_Control(GPIO_TypeDef * GPIO_Port , uint16_t GPIO_Pin , uint8_t cmd);

#endif //FREERTOSMUTEX_LED_H