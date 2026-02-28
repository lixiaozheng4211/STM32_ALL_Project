#include "stm32f10x.h"                  // Device header

int Encoder_Count = 0;

// 初始化pb0 和 pb1 的外部中断
void Encoder_Init(void) {
	// 开启外部时钟并且初始化GPIO口
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// 开启AFIO引脚复用功能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	// 开启外部中断
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);
	
	// EXTI初始化
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line0 | EXTI_Line1;
	EXTI_InitStruct.EXTI_LineCmd =ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger=  EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStruct);
	
	// 优先级分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	// 都是抢占优先级,设置0 - 3
	
	// nvic初始化
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);
}

// 配置0的中断函数
void EXTI0_IRQHandler() {
	if (EXTI_GetITStatus(EXTI_Line0) == SET) {
		if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) == 0) Encoder_Count--;
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

// 配置1的中断函数
void EXTI1_IRQHandler() {
	if (EXTI_GetITStatus(EXTI_Line1) == SET) {
		if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0) == 0) Encoder_Count++;
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}

int Get_Encoder_Count(void) {
	return Encoder_Count;
}


