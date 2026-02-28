#include "stm32f10x.h"                  // Device header

void FM_Init(void) {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}


void FM_ON(void) {
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
}

void FM_OFF(void) {
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

void FM_Turn(void) {
	(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) == 1) ? 
	FM_ON() : FM_OFF();
}
