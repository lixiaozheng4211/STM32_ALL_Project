#include "SPI_Software.h"
#include "gpio.h"

// =============== 软件模拟 SPI (模式 0: CPOL=0, CPHA=0) ==================
/**
 * @brief  软件模拟 SPI 初始化
 * @note   配置 GPIO 并设置总线空闲电平，对应 SPI 模式 0：
 *         - CPOL=0：SCK 空闲时保持低电平
 *         - SS 空闲时保持高电平（未选中从机）
 * @return None
 */
void SPI_Init(void) {
  MX_GPIO_Init();                  // 初始化 SPI 相关 GPIO（SS, SCK, MOSI 推挽输出，MISO 上拉输入）
  SPI_W_SS(1);                     // SS 片选默认拉高（空闲状态，不选中任何从机）
  SPI_W_SCL(0);                    // SCK 时钟默认拉低（对应 CPOL=0，空闲电平为低）
}

/**
 * @brief  启动 SPI 通信（拉低片选）
 * @note   SPI 通信的起始信号：拉低 SS 后，从机开始监听 SCK 和 MOSI
 * @return None
 */
void SPI_Start(void) {
  SPI_W_SS(0);                     // 拉低 SS 片选，选中目标从机，通信开始
}

/**
 * @brief  结束 SPI 通信（拉高片选）
 * @note   SPI 通信的停止信号：拉高 SS 后，从机释放总线，回到空闲状态
 * @return None
 */
void SPI_Stop(void) {
  SPI_W_SS(1);                     // 拉高 SS 片选，释放从机，通信结束（修正原注释笔误）
}

/**
 * @brief  SPI 核心交换函数（全双工，MSB 优先）
 * @note   模拟 SPI 模式 0 时序：
 *         - 先准备 MOSI 数据（SCK 低时更新数据）
 *         - 拉高 SCK（上升沿，从机采样 MOSI，主机采样 MISO）
 *         - 拉低 SCK（回到低电平，准备下一位）
 * @param  byteSend 主机要发送给从机的 1 字节数据
 * @return 从机发送给主机的 1 字节数据
 */
uint8_t SPI_SwapByte(uint8_t byteSend) {
  uint8_t byteReceived = 0x00;     // 初始化接收字节为 0（必须初始化，否则是随机值）

  // 循环 8 次，每次交换 1 位（SPI 默认 MSB 优先：从最高位第 7 位开始，到最低位第 0 位）
  for (uint8_t i = 0; i < 8; i++) {
    // -------------------------- 第一步：准备发送数据（SCK 低时更新 MOSI） --------------------------
    // 技巧：两个 !! 是「布尔归一化」，把任意非 0 值映射为 1，0 值保持 0，避免直接写寄存器时出错
    // 0x80 >> i：生成掩码，依次取出 byteSend 的第 7、6、5...0 位（MSB 优先）
    SPI_W_MOSI(!!(byteSend & (0x80 >> i)));

    // -------------------------- 第二步：拉高 SCK（上升沿，双方采样数据） --------------------------
    SPI_W_SCL(1);                   // SCK 从低变高（上升沿）
                                     // - 从机在此时采样 MOSI 线上的位（CPHA=0：第 1 个沿采样）
                                     // - 主机在此时读取 MISO 线上的位

    // -------------------------- 第三步：读取 MISO 数据 --------------------------
    // 修正原代码小问题：SPI_R_MISO(void) 多了个 void，宏调用不需要参数
    if (SPI_R_MISO) {               // 如果 MISO 线上读到高电平（1）
      byteReceived |= (0x80 >> i);  // 把接收字节的对应位置 1（同样 MSB 优先）
    }

    // -------------------------- 第四步：拉低 SCK（回到空闲电平，准备下一位） --------------------------
    SPI_W_SCL(0);                   // SCK 从高变低（下降沿）
                                     // - 双方在此时准备下一位数据（CPHA=0：第 2 个沿更新）
  }

  return byteReceived;              // 返回完整的 1 字节接收数据
}
// =============== 软件模拟 SPI ==================


