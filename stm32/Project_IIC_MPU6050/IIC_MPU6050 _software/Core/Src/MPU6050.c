#include "MPU6050.h"
// ================================ IIC 程序逻辑 ===========================//
/*
 * 修正后的完整 I2C 写流程
 *
 * 1. Start (起始)
 *    --> SCL 高, SDA 由高变低
 *
 * 2. Send_Address  <-- 你漏掉了这关键一步！
 *    --> 循环8次: SCL低->准备SDA->SCL高->从机读取->SCL低
 *
 * 3. Wait_ACK (等待应答)
 *    --> 主机释放SDA(拉高), 产生第9个时钟, 看SDA是否被从机拉低
 *
 * 4. Send_Byte (发送【寄存器地址】)
 *    --> 告诉从机我要写它内部的哪个寄存器
 *
 * 5. Wait_ACK (等待应答)
 *
 * 6. Send_Byte (发送【真正的数据】)
 *    --> 这才是你要写入寄存器的内容
 *
 * 7. Wait_ACK (等待应答)
 *
 * 8. Stop (停止)
 *    --> SCL 高, SDA 由低变高
 */

// 系统时钟72MHz时，这个循环大约延时几微秒
static void IIC_Delay(void) {
    volatile uint8_t i;
    for (i = 0; i < 10; i++);
}

/**
 * @brief  I2C 起始信号
 * 当 SCL 为高电平时，SDA 由高变低
 */
void IIC_Start(void) {
    IIC_W_SDA(1);
    IIC_W_SCL(1);
    IIC_Delay();
    IIC_W_SDA(0);
    IIC_Delay();
    IIC_W_SCL(0);
}

/**
 * @brief  I2C 停止信号
 * 当 SCL 为高电平时，SDA 由低变高
 */
void IIC_Stop(void) {
    IIC_W_SDA(0);
    IIC_W_SCL(1);
    IIC_Delay();
    IIC_W_SDA(1); // 停止信号：SDA上升沿
    IIC_Delay();
}

/**
 * @brief  I2C 发送一个字节
 * @param  data: 要发送的字节 (8位)
 */
void IIC_Send_Byte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        // 从高位 (MSB) 到低位 (LSB) 依次发送
        IIC_W_SDA(data & 0x80);
        data <<= 1;
        IIC_W_SCL(1);
        IIC_Delay();
        IIC_W_SCL(0);
        IIC_Delay();
    }
}

/**
 * @brief  I2C 等待应答 (ACK)
 * @retval 0: 收到应答 (ACK)
 *         1: 未收到应答 (NACK)
 */
uint8_t IIC_Wait_Ack(void) {
    uint8_t ack_flag;

    IIC_W_SDA(1); // 主机释放SDA，等待从机拉低
    IIC_Delay();
    IIC_W_SCL(1);
    IIC_Delay();
    if (IIC_R_SDA() == GPIO_PIN_SET) ack_flag = 1; // NACK
    else  ack_flag = 0; // ACK
    IIC_W_SCL(0);
    IIC_Delay();
    return ack_flag;
}

/**
 * @brief  I2C 读取一个字节
 * @retval 读到的数据
 */
uint8_t IIC_Read_Byte(void) {
    uint8_t data = 0;
    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1;
        IIC_W_SCL(1);
        IIC_Delay();
        if (IIC_R_SDA()) data |= 0x01; // 读取SDA状态
        IIC_W_SCL(0);
        IIC_Delay();
    }
    return data;
}

/**
 * @brief 接受数据函数
 * @return 返回接受的数据
 */
uint8_t IIC_ReceiveByte(void)
{
    uint8_t Byte = 0;

    // 循环读取 8 位数据
    for (uint8_t i = 0; i < 8; i++)
    {
        // 1. 拉低 SCL，准备让从机发送下一位
        IIC_W_SCL(0);
        IIC_Delay(); // 延时确保从机准备好

        // 2. 数据左移，腾出空间存新的一位
        Byte <<= 1;

        // 3. 拉高 SCL，此时从机必须把数据放到 SDA 上
        IIC_W_SCL(1);
        IIC_Delay();

        // 4. 读取 SDA 引脚状态
        if (IIC_R_SDA() == GPIO_PIN_SET)
        {
            Byte |= 0x01; // 如果是高电平，这一位就是 1
        }
    }

    // 读取完 8 位后，拉低 SCL，结束本次读取
    IIC_W_SCL(0);

    return Byte;
}

/**
 *
 * @param AckBit 发送ACKbit
 */
void IIC_SendAck(uint8_t AckBit)
{
    // 1. 拉低 SCL，主机才能开始控制 SDA
    IIC_W_SCL(0);
    IIC_Delay();

    // 2. 主机写入 SDA
    // AckBit=0: 拉低 SDA (发送应答 ACK)
    // AckBit=1: 拉高 SDA (释放总线，发送非应答 NACK)
    IIC_W_SDA(AckBit);
    IIC_Delay();

    // 3. 拉高 SCL，产生一个时钟脉冲，让从机读取应答
    IIC_W_SCL(1);
    IIC_Delay();

    // 4. 拉低 SCL，结束应答时序
    IIC_W_SCL(0);
}

/**
 *
 * @return 返回接受的ACK
 */
uint8_t IIC_ReceiveAck(void)
{
    uint8_t AckBit;

    // 1. 拉低 SCL
    IIC_W_SCL(0);
    IIC_Delay();

    // 2. 主机释放 SDA (拉高)，把控制权交给从机
    // 这一步非常重要！如果不释放，从机无法拉低 SDA 发送应答
    IIC_W_SDA(1);
    IIC_Delay();

    // 3. 拉高 SCL，准备读取
    IIC_W_SCL(1);
    IIC_Delay();

    // 4. 读取 SDA 状态
    if (IIC_R_SDA() == GPIO_PIN_SET)
    {
        AckBit = 1; // SDA 是高电平，说明从机没应答 (NACK)
    }
    else
    {
        AckBit = 0; // SDA 被从机拉低，应答成功 (ACK)
    }

    // 5. 拉低 SCL，结束
    IIC_W_SCL(0);

    return AckBit;
}
// ================================ IIC 程序逻辑 ===========================//


// ================================ MPU 逻辑 ==============================//
/**
 * @brief 写入寄存器
 * @param RegAddress 指定要写入的寄存器地址
 * @param Data 要输入的数值
 */
void MPU6050_WriteReg(uint8_t RegAddress , uint8_t Data) {
    IIC_Start();
    IIC_Send_Byte(MPU6050_ADDRESS);
    IIC_ReceiveAck();
    IIC_Send_Byte(RegAddress);
    IIC_ReceiveAck();
    IIC_Send_Byte(Data);
    IIC_ReceiveAck();
    IIC_Stop();
}

/**
 * @brief 读取数据寄存器数据
 * @return 返回数据寄存器的数据
 */
uint8_t MPU6050_ReadReg(uint8_t RegAddress) {
    IIC_Start();
    IIC_Send_Byte(MPU6050_ADDRESS);
    IIC_ReceiveAck();
    IIC_Send_Byte(RegAddress);
    IIC_ReceiveAck();
    IIC_Start();
    IIC_Send_Byte(MPU6050_ADDRESS | 0x01);
    IIC_ReceiveAck();
    uint8_t Data = IIC_ReceiveByte();
    IIC_SendAck(1);
    IIC_Stop();
    return Data;
}

/**
 * @brief 初始化MPU
 */
void MPU6050_Init(void) {
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
    MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);
    MPU6050_WriteReg(MPU6050_CONFIG, 0x06);
    MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);
    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);
}

/**
 * @brief 返回使用的MPU地址
 * @return 返回地址数据
 */
uint8_t MPU6050_GetID(void) {
    return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

/**
 * @brief 获取ACC加速器三个方向的数据
 * @param AccX 加速器X轴方向数据
 * @param AccY 加速器Y轴方向数据
 * @param AccZ 加速器Z轴方向数据
 */
void MPU6050_GetAccData(int16_t * AccX , int16_t * AccY , int16_t * AccZ) {
    uint8_t Data_H , Data_L;
    Data_H = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);
    Data_L = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);
    *AccX = (Data_H << 8) | Data_L;
    Data_H = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);
    Data_L = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);
    *AccY = (Data_H << 8) | Data_L;
    Data_H = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);
    Data_L = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);
    *AccZ = (Data_H << 8) | Data_L;
}

/**
 * @brief 获取陀螺仪三个方向的数据
 * @param GyroX 获取陀螺仪X方向数据
 * @param GyroY 获取陀螺仪Y方向数据
 * @param GyroZ 获取陀螺仪Z方向数据
 */
void MPU6050_GetGyroData(int16_t * GyroX , int16_t * GyroY , int16_t * GyroZ) {
    uint8_t Data_H , Data_L;
    Data_H = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);
    Data_L = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);
    *GyroX = (Data_H << 8) | Data_L;
    Data_H = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);
    Data_L = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);
    *GyroY = (Data_H << 8) | Data_L;
    Data_H = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);
    Data_L = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);
    *GyroZ = (Data_H << 8) | Data_L;
}

/**
 * @brief 就是简单OLED显示函数
 */
void OLED_ShowMPUData() {
    int16_t ax , ay , az , gx ,gy ,gz ;
    MPU6050_GetAccData(&ax , &ay , &az );
    MPU6050_GetGyroData(&gx , &gy , &gz );
    OLED_ShowSignedNum(2 , 1 , ax , 4);
    OLED_ShowSignedNum(2 , 8 , gx , 4);
    OLED_ShowSignedNum(3 , 1 , ay , 4);
    OLED_ShowSignedNum(3 , 8 , gy , 4);
    OLED_ShowSignedNum(4 , 1 , az , 4);
    OLED_ShowSignedNum(4 , 8 , gz , 4);
}
// ================================ MPU 逻辑 ==============================//
