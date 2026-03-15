#ifndef FREERTOSQUEUE_LED_CONTROL_H
#define FREERTOSQUEUE_LED_CONTROL_H
#include "stm32f1xx_hal.h"

typedef enum {
  LED_ON = 0,
  LED_OFF = 1,
  LED_TOGGLE = 2,
} LED_Status;

void LED_Control(GPIO_TypeDef * GPIO_Port , uint16_t GPIO_Pin , uint8_t cmd);

#endif //FREERTOSQUEUE_LED_CONTROL_H