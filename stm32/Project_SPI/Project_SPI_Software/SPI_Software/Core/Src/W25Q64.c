#include "w25q64.h"

// ================= 软件SPI底层微秒级延时（根据系统时钟调整，STM32F1 72MHz通用） =================
static void SoftwareSPI_Delay(void) {
    for (volatile uint16_t i = 0; i < 10; i++); // 简单循环延时，约几微秒
}

// ================= 软件SPI核心：读写1个字节（模拟SPI模式0：CPOL=0, CPHA=0, MSB优先） =================
static uint8_t SoftwareSPI_ReadWriteByte(uint8_t TxData) {
    uint8_t RxData = 0;

    for (uint8_t i = 0; i < 8; i++) {
        // 1. 准备数据位：先处理最高位(MSB)
        if (TxData & 0x80) { W25Q64_MOSI_HIGH(); } else { W25Q64_MOSI_LOW(); }
        TxData <<= 1;

        // 2. SCK上升沿：Flash采样MOSI数据
        W25Q64_SCK_HIGH();
        SoftwareSPI_Delay();

        // 3. 读取MISO数据：Flash输出数据
        RxData <<= 1;
        if (W25Q64_MISO_READ()) { RxData |= 0x01; }

        // 4. SCK下降沿：准备下一位
        W25Q64_SCK_LOW();
        SoftwareSPI_Delay();
    }
    return RxData;
}

// ================= 1. 初始化W25Q64的GPIO引脚 =================
void W25DQ64_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1. 使能GPIO时钟（根据端口修改，如__HAL_RCC_GPIOB_CLK_ENABLE()）
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 2. 配置 SCK、MOSI、CS 为推挽输出
    GPIO_InitStruct.Pin = W25Q64_SPI_SCK_PIN | W25Q64_SPI_MOSI_PIN | W25Q64_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(W25Q64_SPI_GPIO_PORT, &GPIO_InitStruct);

    // 3. 配置 MISO 为上拉输入
    GPIO_InitStruct.Pin = W25Q64_SPI_MISO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(W25Q64_SPI_GPIO_PORT, &GPIO_InitStruct);

    // 4. 初始状态：CS拉高(未选中)，SCK拉低(模式0空闲电平)
    W25Q64_CS_HIGH();
    W25Q64_SCK_LOW();
}

// ================= 2. 读取W25Q64的ID（验证硬件连接） =================
void W25DQ64_ReadID(uint8_t *MID, uint16_t *DID) {
    W25Q64_CS_LOW();
    SoftwareSPI_ReadWriteByte(W25Q64_READ_JEDEC_ID); // 发送读ID指令(0x9F)
    *MID = SoftwareSPI_ReadWriteByte(0xFF);            // 读取制造商ID(W25Q64应为0xEF)
    *DID = SoftwareSPI_ReadWriteByte(0xFF);            // 读取设备ID高8位
    *DID <<= 8;
    *DID |= SoftwareSPI_ReadWriteByte(0xFF);           // 读取设备ID低8位(W25Q64应为0x4017)
    W25Q64_CS_HIGH();
}

// ================= 3. 写使能（擦除/写入前必须调用） =================
void W25DQ64_WriteEnable(void) {
    W25Q64_CS_LOW();
    SoftwareSPI_ReadWriteByte(W25Q64_WRITE_ENABLE); // 发送写使能指令(0x06)
    W25Q64_CS_HIGH();
}

// ================= 4. 等待Flash内部操作完成（擦除/写入后必须等待） =================
void W25DQ64_WaitBusy(void) {
    W25Q64_CS_LOW();
    SoftwareSPI_ReadWriteByte(W25Q64_READ_STATUS_REG1); // 发送读状态寄存器指令(0x05)
    // 循环读取状态寄存器，等待BUSY位(位0)变为0(1=忙, 0=空闲)
    while (SoftwareSPI_ReadWriteByte(0xFF) & 0x01);
    W25Q64_CS_HIGH();
}

// ================= 5. 页编程（写入数据，最大256字节，不可跨页） =================
void W25DQ64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t count) {
    W25DQ64_WriteEnable(); // 1. 写使能解锁
    W25Q64_CS_LOW();
    SoftwareSPI_ReadWriteByte(W25Q64_PAGE_PROGRAM);              // 2. 发送页编程指令(0x02)
    SoftwareSPI_ReadWriteByte((Address >> 16) & 0xFF);           // 3. 发送24位地址(高8位)
    SoftwareSPI_ReadWriteByte((Address >> 8) & 0xFF);            // 4. 发送地址(中8位)
    SoftwareSPI_ReadWriteByte(Address & 0xFF);                    // 5. 发送地址(低8位)
    for (uint16_t i = 0; i < count; i++) {
        SoftwareSPI_ReadWriteByte(DataArray[i]);                  // 6. 循环写入数据
    }
    W25Q64_CS_HIGH();
    W25DQ64_WaitBusy(); // 7. 等待编程完成
}

// ================= 6. 扇区擦除（4KB，擦除后数据全为0xFF） =================
void W25DQ64_EraseSector(uint32_t Address) {
    W25DQ64_WriteEnable(); // 1. 写使能解锁
    W25Q64_CS_LOW();
    SoftwareSPI_ReadWriteByte(W25Q64_SECTOR_ERASE_4KB);         // 2. 发送扇区擦除指令(0x20)
    SoftwareSPI_ReadWriteByte((Address >> 16) & 0xFF);           // 3. 发送24位地址
    SoftwareSPI_ReadWriteByte((Address >> 8) & 0xFF);
    SoftwareSPI_ReadWriteByte(Address & 0xFF);
    W25Q64_CS_HIGH();
    W25DQ64_WaitBusy(); // 4. 等待擦除完成(约几十毫秒)
}

// ================= 7. 读取数据（支持连续读取任意长度） =================
void W25DQ64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count) {
    W25Q64_CS_LOW();
    SoftwareSPI_ReadWriteByte(W25Q64_READ_DATA);                 // 1. 发送读数据指令(0x03)
    SoftwareSPI_ReadWriteByte((Address >> 16) & 0xFF);           // 2. 发送24位地址
    SoftwareSPI_ReadWriteByte((Address >> 8) & 0xFF);
    SoftwareSPI_ReadWriteByte(Address & 0xFF);
    for (uint32_t i = 0; i < Count; i++) {
        DataArray[i] = SoftwareSPI_ReadWriteByte(0xFF);          // 3. 循环读取数据
    }
    W25Q64_CS_HIGH();
}