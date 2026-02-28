#include "stm32f10x.h"                  // Device header

void PWM_Init(void) {
	// 梳理配置PWM的流程
	// 1 系统时钟/RCC初始化
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE); // 打开时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE); // 开启引脚复用功能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // 开启PA0GPIO
	// 2 定时器基础配置 -> 设置频率
	// 配置时基单元
	TIM_InternalClockConfig(TIM2); // 选择内部时钟
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct; //初始化时基单元
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟分频模式一般是选择1不分频
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 10000 - 1; ;// 这就是ARR 自动重载寄存器
	TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1; // 分频器的分频系数
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0; // 就是重复计数器的值,高级定时器才有这里暂时不用
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	
	// 配置GPIO
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; // 使用复用推挽输出这是因为定时器是片上外设所以使用复用功能输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// 3 GPIO引脚复用配置(硬件输出) 初始化输出比较单元
	TIM_OCInitTypeDef TIM_OCInitStruct; // 注意有些东西是高级定时器才会用到的功能,所以现在只需要默认初始化就可以
	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1; // 设置输出比较模式
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High; // 设置输出比较极性 , 就是说是不是要反转电平
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; // 设置输出使能
	TIM_OCInitStruct.TIM_Pulse = 0; // 设置 CCR 就是(capture / compare register) 作用是:判断与CNT的大小就是占空比
	// 在程序运行的过程重配置这个CCR的值,现在就可以先设置成0
	// 默认复用功能就是偏上外设与GPIO的关系
	// 使用TIM2_CH1_ETR借用在AP0的gpio口上,还可以重定义在PA15上
	TIM_OC1Init(TIM2, &TIM_OCInitStruct); // TIM_OC1 通道初始初始化
	TIM_Cmd(TIM2,ENABLE); // 启动定时器
}

void PWM_SetCompare1(uint16_t Compare) {
	TIM_SetCompare1(TIM2,Compare);
}
