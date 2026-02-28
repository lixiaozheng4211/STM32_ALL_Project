#ifndef __W25Q64_H
#define __W25Q64_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"  // 根据你的STM32系列修改（如f4xx, f7xx等）
#include <stdint.h>

// ================= 软件SPI引脚配置（请根据实际电路修改） =================
#define W25Q64_SPI_SCK_PIN     GPIO_PIN_5    // SCK引脚
#define W25Q64_SPI_MISO_PIN    GPIO_PIN_6    // MISO引脚
#define W25Q64_SPI_MOSI_PIN    GPIO_PIN_7    // MOSI引脚
#define W25Q64_CS_PIN           GPIO_PIN_4    // CS引脚
#define W25Q64_SPI_GPIO_PORT    GPIOA         // GPIO端口

// ================= W25Q64 Flash指令集 =================
#define W25Q64_WRITE_ENABLE          0x06    // 写使能
#define W25Q64_READ_STATUS_REG1      0x05    // 读状态寄存器1
#define W25Q64_PAGE_PROGRAM          0x02    // 页编程（写入）
#define W25Q64_SECTOR_ERASE_4KB      0x20    // 扇区擦除（4KB）
#define W25Q64_READ_DATA             0x03    // 读数据
#define W25Q64_READ_JEDEC_ID         0x9F    // 读JEDEC ID

// ================= GPIO操作简化宏（提高代码可读性） =================
#define W25Q64_CS_LOW()     HAL_GPIO_WritePin(W25Q64_SPI_GPIO_PORT, W25Q64_CS_PIN, GPIO_PIN_RESET)
#define W25Q64_CS_HIGH()    HAL_GPIO_WritePin(W25Q64_SPI_GPIO_PORT, W25Q64_CS_PIN, GPIO_PIN_SET)
#define W25Q64_SCK_LOW()    HAL_GPIO_WritePin(W25Q64_SPI_GPIO_PORT, W25Q64_SPI_SCK_PIN, GPIO_PIN_RESET)
#define W25Q64_SCK_HIGH()   HAL_GPIO_WritePin(W25Q64_SPI_GPIO_PORT, W25Q64_SPI_SCK_PIN, GPIO_PIN_SET)
#define W25Q64_MOSI_LOW()   HAL_GPIO_WritePin(W25Q64_SPI_GPIO_PORT, W25Q64_SPI_MOSI_PIN, GPIO_PIN_RESET)
#define W25Q64_MOSI_HIGH()  HAL_GPIO_WritePin(W25Q64_SPI_GPIO_PORT, W25Q64_SPI_MOSI_PIN, GPIO_PIN_SET)
#define W25Q64_MISO_READ()  HAL_GPIO_ReadPin(W25Q64_SPI_GPIO_PORT, W25Q64_SPI_MISO_PIN)

// ================= 函数声明 =================
void    W25DQ64_Init(void);
void    W25DQ64_ReadID(uint8_t *MID, uint16_t *DID);
void    W25DQ64_WriteEnable(void);
void    W25DQ64_WaitBusy(void);
void    W25DQ64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t count);
void    W25DQ64_EraseSector(uint32_t Address);
void    W25DQ64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count);

#ifdef __cplusplus
}
#endif

#endif /* __W25Q64_H */