#ifndef SPI_HARDWARE_W25Q64_H
#define SPI_HARDWARE_W25Q64_H

// 包含头文件
#include "stm32f1xx_hal.h"
#include "stdint.h"
#include "W25Q64_Ins.h"

// 宏定义
#define SPI1_SS_Pin GPIO_PIN_4
#define SPI1_SS_GPIO_Port GPIOA

#define W25DQ64_CS_LOW(void) HAL_GPIO_WritePin(SPI1_SS_GPIO_Port, SPI1_SS_Pin, GPIO_PIN_RESET)
#define W25DQ64_CS_HIGH(void) HAL_GPIO_WritePin(SPI1_SS_GPIO_Port, SPI1_SS_Pin, GPIO_PIN_SET)


// 函数声明
void W25DQ64_Init(void);
void W25DQ64_ReadID(uint8_t * MID , uint16_t * DID);
void W25DQ64_WriteEnable(void);
void W25DQ64_WaitBusy(void);
void W25DQ64_PageProgram(uint32_t Address, uint8_t* DataArray , uint16_t count);
void W25DQ64_EraseSector(uint32_t Address);
void W25DQ64_ReadData(uint32_t Adress, uint8_t * DataArray , uint32_t );


#endif //SPI_HARDWARE_W25Q64_H