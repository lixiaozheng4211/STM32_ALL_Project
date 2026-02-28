#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include "stm32f1xx_hal.h"

// 使用extern声明在main.c中定义的I2C句柄
extern I2C_HandleTypeDef hi2c1;

// 函数声明
void OLED_I2C_Init(void);
void OLED_WriteCommand(uint8_t Command);
void OLED_WriteData(uint8_t Data);
void OLED_SetCursor(uint8_t Y, uint8_t X);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_Init(void);

// 设备地址定义
#define OLED_ADDRESS 0x78  // OLED的I2C地址
#define OLED_CMD 0x00     // 写命令
#define OLED_DATA 0x40    // 写数据

#endif