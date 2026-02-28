// 头文件保护：防止同一个头文件被多次包含，避免重复定义错误
#ifndef __LED_INIT_H
#define __LED_INIT_H

// 依赖头文件：包含STM32核心头文件，确保GPIO相关定义可用
#include "stm32f10x.h"

/************************* LED硬件宏定义 *************************/
// 把LED的硬件信息（端口、引脚、时钟）用宏定义，后期改硬件不用改函数
// LED1 配置
#define LED1_GPIO_PORT    GPIOB
#define LED1_GPIO_PIN     GPIO_Pin_0
#define LED1_GPIO_CLK     RCC_APB2Periph_GPIOB

// LED2 配置
#define LED2_GPIO_PORT    GPIOB
#define LED2_GPIO_PIN     GPIO_Pin_1
#define LED2_GPIO_CLK     RCC_APB2Periph_GPIOB

/************************* 函数声明（带注释） *************************/
/**
 * @brief  初始化LED对应的GPIO引脚（推挽输出模式）
 * @param  无
 * @retval 无
 */
void LED_Init(void);

/**
 * @brief  点亮LED1
 * @param  无
 * @retval 无
 */
void LED1_ON(void);

/**
 * @brief  熄灭LED1
 * @param  无
 * @retval 无
 */
void LED1_OFF(void);

/**
 * @brief  翻转LED1的亮灭状态
 * @param  无
 * @retval 无
 */
void LED1_Turn(void);

/**
 * @brief  点亮LED2
 * @param  无
 * @retval 无
 */
void LED2_ON(void);

/**
 * @brief  熄灭LED2
 * @param  无
 * @retval 无
 */
void LED2_OFF(void);

/**
 * @brief  翻转LED2的亮灭状态
 * @param  无
 * @retval 无
 */
void LED2_Turn(void);

#endif /* __LED_INIT_H */
