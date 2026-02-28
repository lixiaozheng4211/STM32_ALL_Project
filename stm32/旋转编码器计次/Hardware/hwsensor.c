#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include "Delay.h"
uint16_t CountSensor_Count = 0;

void Hw_Sensor_Init(void) {
	// 第一步:配置外设时钟 以及第二步配置GPIO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	// 开启AFIO时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	// 配置GPIO
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
	// 第三步配置AFIO
	//GPIO_PinRemapConfig();// 进行引脚重映射
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB ,GPIO_PinSource14);// 配置AFIO数据选择器来选择想要的中断引脚
	// 表示连接PB14号口的第14个中断线路
	// 第四步配置exti(外部中断)函数
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line14;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStruct);
	
	// 第五步配置nvic,分组方式,整个工程只需要一次就可以,就是设置
	// 抢占优先级和响应优先级的数量
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	// 使用结构体初始化NVIC
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
}

void EXTI15_10_IRQHandler(void) {
	if (EXTI_GetITStatus(EXTI_Line14) == SET) {
		CountSensor_Count++;
		LED_ON();
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14) == 1);
		LED_OFF();
		// 每次中断函数结束后都要清除中断函数标志位,不然一直卡在这里
		EXTI_ClearITPendingBit(EXTI_Line14);
	} 
}


uint16_t Count_Sensor_GET(void) {
	return CountSensor_Count;
}
