#ifndef SPI_SOFTWARE_SPI_SOFTWARE_H
#define SPI_SOFTWARE_SPI_SOFTWARE_H
// =================== userInclude ==================
#include "OLED.h"
#include "stm32f1xx_hal.h"
// =================== userInclude ==================

// =================== userDefine ===================
#define SPI_CS_Pin GPIO_PIN_4
#define SPI_CS_GPIO_Port GPIOA
#define SPI_SCL_Pin GPIO_PIN_5
#define SPI_SCL_GPIO_Port GPIOA
#define SPI_DO_Pin GPIO_PIN_6
#define SPI_DO_GPIO_Port GPIOA
#define SPI_DI_Pin GPIO_PIN_7
#define SPI_DI_GPIO_Port GPIOA
#define OLED_SCL_Pin GPIO_PIN_8
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_9
#define OLED_SDA_GPIO_Port GPIOB

/**
 * @brief 设置片选引脚的电平
 * @param BitValue 表示使用的电平 , 是高电平还是低电平
 */
#define SPI_W_SS(BitValue) HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin , BitValue)

/**
 * @brief SPI写 SCK 的引脚电平
 * @param BitValue 要写入的电平值
 */
#define SPI_W_SCL(BitValue) HAL_GPIO_WritePin(SPI_SCL_GPIO_Port, SPI_SCL_Pin , BitValue)

/**
 * @brief SPI 写MOSI引脚电平
 * @param BitValue 传入当前需要写的MOSI电平
 */
#define SPI_W_MOSI(BitValue) HAL_GPIO_WritePin(SPI_DI_GPIO_Port, SPI_DI_Pin , BitValue)

/**
 * @brief SPI 读取MISO的电平
 * @retval 读取的引脚的电平
 */
#define SPI_R_MISO HAL_GPIO_ReadPin(SPI_DO_GPIO_Port , SPI_DO_Pin)
// =================== userDefine ===================

// =================== SPI ==========================
void SPI_Init(void);
void SPI_Start(void);
void SPI_Stop(void);
uint8_t SPI_SwapByte(uint8_t byte);
// =================== SPI ==========================

#endif //SPI_SOFTWARE_SPI_SOFTWARE_H