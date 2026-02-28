#include "W25Q64.h"
#include "spi.h"

/**
 * @brief  W25Q64 初始化函数
 * @note   仅初始化片选引脚默认状态，确保上电后芯片处于未选中状态
 */
void W25DQ64_Init(void) {
  W25DQ64_CS_HIGH();  // SPI片选默认拉高（低电平有效），避免误触发
}

/**
 * @brief  读取 W25Q64 的 JEDEC ID
 * @param  MID: 指针，返回厂商ID（Winbond固定为 0xEF）
 * @param  DID: 指针，返回设备ID（W25Q64为 0x4017）
 * @note   通信流程：CS拉低 → 发0x9F指令 → 读3字节 → CS拉高
 */
void W25DQ64_ReadID(uint8_t *MID, uint16_t *DID) {
  uint8_t cmd = W25Q64_JEDEC_ID;  // JEDEC ID读取指令：0x9F
  uint8_t response[3];             // 接收缓冲区：[0]厂商ID, [1]设备ID高8位, [2]设备ID低8位

  W25DQ64_CS_LOW();                                    // 1. 拉低片选，选中芯片
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 1000);           // 2. 发送读取ID指令
  HAL_SPI_Receive(&hspi1, response, 3, 1000);         // 3. 连续读取3字节ID数据
  W25DQ64_CS_HIGH();                                   // 4. 拉高片选，结束通信

  *MID = response[0];                                  // 提取厂商ID
  *DID = (response[1] << 8) | response[2];            // 拼接16位设备ID
}

/**
 * @brief  W25Q64 写使能函数
 * @note   执行“页编程/扇区擦除/块擦除/全片擦除”前，必须先调用此函数
 */
void W25DQ64_WriteEnable(void) {
  uint8_t cmd = W25Q64_WRITE_ENABLE;  // 写使能指令：0x06

  W25DQ64_CS_LOW();                          // 1. 拉低片选
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 1000); // 2. 发送写使能指令
  W25DQ64_CS_HIGH();                         // 3. 拉高片选（指令在CS上升沿生效）
}

/**
 * @brief  等待 W25Q64 内部操作完成（忙等待）
 * @note   通过读取状态寄存器1的BUSY位(bit0)判断：1=忙，0=空闲
 */
void W25DQ64_WaitBusy(void) {
  uint32_t TimeOut = 1000000;  // 超时计数器（防止硬件异常卡死）
  uint8_t cmd = W25Q64_READ_STATUS_REGISTER_1; // 读状态寄存器1指令：0x05
  uint8_t status;

  W25DQ64_CS_LOW();                          // 1. 拉低片选（保持低电平以连续读取状态）
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 1000); // 2. 发送读状态寄存器指令

  // 3. 循环读取状态，直到BUSY位为0或超时
  do {
    HAL_SPI_Receive(&hspi1, &status, 1, 1000); // 每次循环都读取最新状态
    TimeOut--;
    if (TimeOut == 0) break;                     // 超时强制退出
  } while ((status & 0x01) == 0x01);            // 判断BUSY位(bit0)

  W25DQ64_CS_HIGH();                         // 4. 拉高片选，结束通信
}

/**
 * @brief  W25Q64 页编程函数（写入数据）
 * @param  Address: 24位起始写入地址（范围：0x000000 ~ 0x7FFFFF）
 * @param  DataArray: 指向要写入数据的指针
 * @param  count: 要写入的字节数（**最大256字节，且不能跨页边界**）
 * @note   页边界：地址低8位为0x00的位置（如0x100, 0x200...），跨页需分多次写入
 */
void W25DQ64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t count) {
  // 发送缓冲区：[0]页编程指令, [1]地址A23-A16, [2]地址A15-A8, [3]地址A7-A0
  uint8_t cmd[4] = {
    W25Q64_PAGE_PROGRAM,
    (uint8_t)((Address >> 16) & 0xFF),
    (uint8_t)((Address >> 8) & 0xFF),
    (uint8_t)(Address & 0xFF)
  };

  W25DQ64_WriteEnable();                   // 1. 先发送写使能指令

  W25DQ64_CS_LOW();                          // 2. 拉低片选
  HAL_SPI_Transmit(&hspi1, cmd, 4, 1000);   // 3. 发送“页编程指令 + 24位地址”
  HAL_SPI_Transmit(&hspi1, DataArray, count, 1000); // 4. 发送要写入的数据（修正：用Transmit而非Receive）
  W25DQ64_CS_HIGH();                         // 5. 拉高片选（W25Q64在CS上升沿开始内部写入）

  W25DQ64_WaitBusy();                        // 6. 等待内部写入操作完成
}

void W25DQ64_EraseSector(uint32_t Address) {
  uint8_t cmd[4] = {
    W25Q64_SECTOR_ERASE_4KB,
    (uint8_t)((Address >> 16) & 0xFF),
    (uint8_t)((Address >> 8) & 0xFF),
    (uint8_t)(Address & 0xFF)
  };
  W25DQ64_WriteEnable();
  W25DQ64_CS_LOW(); // 选中芯片
  HAL_SPI_Transmit(&hspi1, cmd, 4, 1000);
  W25DQ64_CS_HIGH();
  W25DQ64_WaitBusy();
}

void W25DQ64_ReadData(uint32_t Adress, uint8_t * DataArray , uint32_t Count) {
  uint8_t cmd[4] = {
    W25Q64_READ_DATA,
    (uint8_t)((Adress >> 16) & 0xFF),
    (uint8_t)((Adress >> 8) & 0xFF),
    (uint8_t)(Adress & 0xFF)
  };
  W25DQ64_CS_LOW();
  HAL_SPI_Transmit(&hspi1, cmd, 4, 1000);
  HAL_SPI_Receive(&hspi1, DataArray, Count, 1000);
  W25DQ64_CS_HIGH();
}