#ifndef __IIC_SOFTWARE_H
#define __IIC_SOFTWARE_H

#ifdef __cplusplus
extern "C" {
#endif

// 依赖HAL库，必须包含
#include "stm32f1xx_hal.h"
#include <stdint.h>


#define SOFT_I2C_SCL_PORT    GPIOB
#define SOFT_I2C_SCL_PIN     GPIO_PIN_8
#define SOFT_I2C_SDA_PORT    GPIOB
#define SOFT_I2C_SDA_PIN     GPIO_PIN_9

// I2C速度配置
#define SOFT_I2C_DELAY_US    2
// 应答等待超时时间
#define SOFT_I2C_ACK_TIMEOUT 0


// ===================== 底层GPIO操作宏（无需修改） =====================
// SCL电平控制
#define SOFT_I2C_SCL_H()  HAL_GPIO_WritePin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SCL_L()  HAL_GPIO_WritePin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN, GPIO_PIN_RESET)

// SDA电平控制
#define SOFT_I2C_SDA_H()  HAL_GPIO_WritePin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SDA_L()  HAL_GPIO_WritePin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN, GPIO_PIN_RESET)

// SDA电平读取
#define SOFT_I2C_SDA_READ()  HAL_GPIO_ReadPin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN)

// SDA模式切换：软件I2C核心，写数据用推挽输出，读应答用上拉输入
#define SOFT_I2C_SDA_OUTPUT() \
  do { \
    GPIO_InitTypeDef GPIO_InitStruct = {0}; \
    GPIO_InitStruct.Pin = SOFT_I2C_SDA_PIN; \
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; \
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; \
    HAL_GPIO_Init(SOFT_I2C_SDA_PORT, &GPIO_InitStruct); \
  } while(0)

#define SOFT_I2C_SDA_INPUT() \
  do { \
    GPIO_InitTypeDef GPIO_InitStruct = {0}; \
    GPIO_InitStruct.Pin = SOFT_I2C_SDA_PIN; \
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; \
    GPIO_InitStruct.Pull = GPIO_PULLUP; \
    HAL_GPIO_Init(SOFT_I2C_SDA_PORT, &GPIO_InitStruct); \
  } while(0)
// ============================================================================

// ===================== 对外接口函数声明 =====================
// 基础初始化
void Soft_I2C_Init(void);

// I2C核心时序函数
void Soft_I2C_Start(void);       // 起始信号
void Soft_I2C_Stop(void);        // 停止信号
uint8_t Soft_I2C_Wait_Ack(void); // 等待从机应答（返回0=应答成功，1=应答失败）
void Soft_I2C_Send_Ack(void);    // 主机发送应答（继续读数据用）
void Soft_I2C_Send_NAck(void);   // 主机发送非应答（结束读数据用）

// 单字节读写
void Soft_I2C_Write_Byte(uint8_t data);  // 主机写1个字节
uint8_t Soft_I2C_Read_Byte(uint8_t ack); // 主机读1个字节（ack=1发送应答，ack=0发送非应答）

// 通用封装函数（上层OLED/传感器直接调用）
// 写单个寄存器：dev_addr=7位设备地址，reg_addr=寄存器地址，data=要写入的数据
uint8_t Soft_I2C_Write_Reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
// 读单个寄存器：dev_addr=7位设备地址，reg_addr=寄存器地址，*pdata=读取数据的存放地址
uint8_t Soft_I2C_Read_Reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *pdata);
// 连续写多个字节：dev_addr=7位设备地址，reg_addr=起始寄存器地址，*pbuf=数据缓冲区，len=数据长度
uint8_t Soft_I2C_Write_Buffer(uint8_t dev_addr, uint8_t reg_addr, uint8_t *pbuf, uint16_t len);
// 连续读多个字节：dev_addr=7位设备地址，reg_addr=起始寄存器地址，*pbuf=数据缓冲区，len=数据长度
uint8_t Soft_I2C_Read_Buffer(uint8_t dev_addr, uint8_t reg_addr, uint8_t *pbuf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // __IIC_SOFTWARE_H