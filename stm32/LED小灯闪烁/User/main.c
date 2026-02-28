#include "stm32f10x.h"                  // Device header
#include "Delay.h"


int main(void) {
	// 使用RCC开启GPIO的时钟
	/*
	都是常用函数
	void RCC_AHBPeriphClockCmd(uint32_t RCC_AHBPeriph, FunctionalState NewState);
	void RCC_APB2PeriphClockCmd(uint32_t RCC_APB2Periph, FunctionalState NewState);
	void RCC_APB1PeriphClockCmd(uint32_t RCC_APB1Periph, FunctionalState NewState);
	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	// 使用GPIO_INIT函数初始化GPIO
	// 就是初始化一个结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	// 控制GPIO口
	while(1) {
		GPIO_WriteBit(GPIOA,GPIO_Pin_0 , Bit_RESET);
		Delay_ms(500);
		GPIO_WriteBit(GPIOA,GPIO_Pin_0 , Bit_SET);
		Delay_ms(500);
	}
}
