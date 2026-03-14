#ifndef FREERTOSLEDEXPERIMENT_LED_H
#define FREERTOSLEDEXPERIMENT_LED_H
#include "main.h"
#define LED_ON      1
#define LED_OFF     0
#define LED_Toggle  2

void LED_Control(GPIO_TypeDef * GPIO_Port , uint16_t GPIO_Pin , uint8_t cmd);
#endif //FREERTOSLEDEXPERIMENT_LED_H