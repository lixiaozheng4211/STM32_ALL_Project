#include "stm32f10x.h"                  // Device header

void PWM_Init(void) {
    // 1. 开启时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // TIM2属于APB1总线外设，需开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // PA2是TIM2_CH3复用引脚，开启GPIOA时钟
    
    // 2. 配置定时器时基单元（核心：决定PWM频率）
    TIM_InternalClockConfig(TIM2); // 使用定时器内部时钟（72MHz，STM32F10x默认系统时钟）
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟不分频
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式（PWM常用）
    TIM_TimeBaseInitStruct.TIM_Period = 20000 - 1; // ARR=19999（自动重装值）
    TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1; // PSC=71（预分频值）
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0; // 通用定时器（TIM2）无需重复计数，设0
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    
    // 3. 配置GPIO（PA2复用为TIM2_CH3）
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出（必须！PWM信号由定时器控制，非普通GPIO）
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2; // TIM2_CH3对应PA2引脚
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // 输出速度50MHz
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 4. 配置输出比较单元（PWM核心规则）
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCStructInit(&TIM_OCInitStruct); // 初始化结构体（避免未赋值成员导致错误）
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1; // PWM模式1（最常用）
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High; // 有效电平为高电平
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; // 启用输出比较（允许PWM输出）
    TIM_OCInitStruct.TIM_Pulse = 0; // 初始比较值=0（占空比0%，电机初始不转）
    TIM_OC3Init(TIM2, &TIM_OCInitStruct); // 初始化TIM2通道3
    
    // 5. 启动定时器（开始输出PWM）
    TIM_Cmd(TIM2,ENABLE);
}

void PWM_SetCompare3(uint16_t Compare) {
	TIM_SetCompare3(TIM2, Compare);
}
