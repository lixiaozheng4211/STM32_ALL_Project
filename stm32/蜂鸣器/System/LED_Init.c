#include "stm32f10x.h"                  // Device header

void LED_Init(void)
{
	// 就是初始化时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  // 初始化GPIO初始化结构体
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 设置是推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2; // 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_SetBits(GPIOA, GPIO_Pin_1 | GPIO_Pin_2);
}

void LED1_ON(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

void LED1_OFF(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
}

void LED1_Turn(void) {
     (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_1) == 0) ?    
			GPIO_SetBits(GPIOA, GPIO_Pin_1) :
			GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

void LED2_ON(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}

void LED2_OFF(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_2);
}

void LED2_Turn(void) {
     (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2) == 0) ?    
			GPIO_SetBits(GPIOA, GPIO_Pin_2) :
			GPIO_ResetBits(GPIOA, GPIO_Pin_2);
}
