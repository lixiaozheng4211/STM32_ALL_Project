#ifndef __MPU6050_H
#define __MPU6050_H
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "MPU6050_Reg.h"
#include "OLED.h"

#define IIC_SCL_PIN GPIO_PIN_10
#define IIC_SDA_PIN GPIO_PIN_11
#define IIC_GPIO_PORT GPIOB


// 写 SCL 引脚 (1:高电平, 0:低电平)
#define IIC_W_SCL(x)     HAL_GPIO_WritePin(IIC_GPIO_PORT, IIC_SCL_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
// 写 SDA 引脚
#define IIC_W_SDA(x)     HAL_GPIO_WritePin(IIC_GPIO_PORT, IIC_SDA_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
// 读 SDA 引脚 (用于接收应答和数据)
#define IIC_R_SDA()      HAL_GPIO_ReadPin(IIC_GPIO_PORT, IIC_SDA_PIN)

// ================================ IIC ==============================//
static void IIC_Delay(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_Send_Byte(uint8_t data);
uint8_t IIC_Wait_Ack(void);
uint8_t IIC_Read_Byte(void);
uint8_t IIC_ReceiveByte(void);
void IIC_SendAck(uint8_t AckBit);
uint8_t IIC_ReceiveAck(void);
// ================================ IIC ==============================//



// ================================ MPU ==============================//
// 这些都是看的数据手册 , 不知道为什么要这么设置就去看看MPU6050的数据手册
void MPU6050_WriteReg(uint8_t RegAddress , uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t RegAddress);
void MPU6050_Init(void);
uint8_t MPU6050_GetID(void);
void MPU6050_GetAccData(int16_t * AccX , int16_t * AccY , int16_t * AccZ);
void MPU6050_GetGyroData(int16_t * GyroX , int16_t * GyroY , int16_t * GyroZ);
void OLED_ShowMPUData();
// ================================ MPU ==============================//

#endif
