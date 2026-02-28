#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H

#include "tim.h"
#include "gpio.h"

typedef enum {
    ENC_EVT_NONE = 0,
    ENC_EVT_CW,
    ENC_EVT_CCW,
    ENC_EVT_SHORT_PRESS,
    ENC_EVT_LONG_PRESS
} Encoder_Event_t;

void BSP_Encoder_Init(TIM_HandleTypeDef *htim);
void BSP_Encoder_Scan(void);
Encoder_Event_t BSP_Encoder_GetEvent(void);

#endif
