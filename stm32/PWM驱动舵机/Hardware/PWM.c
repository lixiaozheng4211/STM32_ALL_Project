#include "stm32f10x.h"                  // Device header

void PWM_Init(void) {
	// 1. 开启时钟（修正错误+补充遗漏）
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // TIM2是APB1外设
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 开启GPIOA时钟（PA0）
	
	// 2. 配置定时器时基单元（10kHz PWM）
	TIM_InternalClockConfig(TIM2); 
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 20000 - 1; // ARR=999（10kHz）
	TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1; // PSC=71
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	
	// 3. 配置GPIO（PA0复用推挽输出）
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// 4. 配置输出比较单元
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = 0; // 初始占空比0%
	TIM_OC2Init(TIM2, &TIM_OCInitStruct);
	
	// 5. 启动定时器
	TIM_Cmd(TIM2,ENABLE);
}

void PWM_SetCompare2(uint16_t Compare) {
	TIM_SetCompare2(TIM2, Compare);
}
