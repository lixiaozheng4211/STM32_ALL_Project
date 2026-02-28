#include "i2c.h"
#include "MPU6050.h"
// 宏定义
#define MPU6050_ADDR 0xD0 // MPU6050的I2C地址 , 0x78 << 1 || 0x00
#define PWR_MGMT_1 0x6B // 电源管理寄存器地址
#define ACCEL_XOUT_H 0x3B // 加速度计X轴高位地址
#define GYRO_XOUT_H 0x43 // 陀螺仪X轴高位寄存器地址
#define MPU6050_WHO_AM_I_REG 0x75 // 设备ID寄存器地址

/**
 * @brief 初始化MPU
 * @param hi2c
 * @retval None
 */
void MPU6050_Init(I2C_HandleTypeDef * hi2c) {
  // 唤醒MPU
  HAL_I2C_Mem_Write(hi2c , MPU6050_ADDR , PWR_MGMT_1 , 1 , (uint8_t*)0x00 , 1 , HAL_MAX_DELAY);
  // 设置采样分辨率
  HAL_I2C_Mem_Write(hi2c , MPU6050_ADDR , 0x19 , 1 , (uint8_t*)0x07 , 1 , HAL_MAX_DELAY);
  // 低通滤波器
  HAL_I2C_Mem_Write(hi2c , MPU6050_ADDR , 0x1A , 1 , (uint8_t*)0x06 , 1 , HAL_MAX_DELAY);
  // 陀螺仪角速度
  HAL_I2C_Mem_Write(hi2c , MPU6050_ADDR , 0x1B , 1 , (uint8_t*)0x08 , 1 , HAL_MAX_DELAY);
  // 加速度计
  HAL_I2C_Mem_Write(hi2c , MPU6050_ADDR , 0x1C , 1 , (uint8_t*)0x00 , 1 , HAL_MAX_DELAY);
}


/**
 * @brief 读取MPU6050的16位数据
 * @param hi2c IIC句柄
 * @param reg 要读取的寄存器地址
 * @retval 读取的16位数据
 */
int16_t MPU6050_ReadData( I2C_HandleTypeDef * hi2c , uint8_t reg) {
  uint8_t data[2];
  HAL_I2C_Mem_Read(hi2c , MPU6050_ADDR , reg , 1 , (uint8_t*)data , 2 , HAL_MAX_DELAY);
  return (int16_t)(data[0] << 8 | data[1]);
}


/**
 * @brief 读取MPU的设备ID号
 * @param hi2c 使用的I2C句柄
 * @return 16进制的设备ID号
 */
uint8_t MPU6050_ReadDeviceID(I2C_HandleTypeDef * hi2c) {
  uint8_t device_id;
  HAL_I2C_Mem_Read(hi2c , MPU6050_ADDR , MPU6050_WHO_AM_I_REG , 1 , (uint8_t*)&device_id, 1 , HAL_MAX_DELAY);
  return device_id;
}

/**
 * @brief 读取三个方向的加速度
 * @param hi2c IIC句柄
 * @param ax X轴方向加速度
 * @param ay Y轴方向加速度
 * @param az Z轴方向加速度
 */
void MPU6050_ReadAccel(I2C_HandleTypeDef * hi2c, int16_t * ax, int16_t * ay, int16_t * az) {
  *ax = MPU6050_ReadData(hi2c , ACCEL_XOUT_H);
  *ay = MPU6050_ReadData(hi2c , ACCEL_XOUT_H + 2);
  *az = MPU6050_ReadData(hi2c , ACCEL_XOUT_H + 4);
}


/**
 * @brief 读取陀螺仪数据
 * @param hi2c IIC句柄
 * @param gx 陀螺仪X方向数据
 * @param gy 陀螺仪Y方向数据
 * @param gz 陀螺仪Z方向数据
 */
void MPU6050_ReadGyro(I2C_HandleTypeDef * hi2c, int16_t * gx, int16_t * gy, int16_t * gz) {
  *gx = MPU6050_ReadData(hi2c , GYRO_XOUT_H);
  *gy = MPU6050_ReadData(hi2c , GYRO_XOUT_H + 2);
  *gz = MPU6050_ReadData(hi2c , GYRO_XOUT_H + 4);
}

void OLED_ShowMPU6050Data(void) {
  uint8_t device_id = MPU6050_ReadDeviceID(&hi2c2);
  int16_t ax, ay, az , gx , gy , gz;
  MPU6050_ReadAccel(&hi2c2 , &ax , &ay , &az);
  MPU6050_ReadGyro(&hi2c2 , &gx , &gy , &gz);
  OLED_ShowHexNum(1 , 5 , device_id , 2);
  OLED_ShowSignedNum(2 , 1 , ax , 4);
  OLED_ShowSignedNum(2 , 8 , gx , 4);
  OLED_ShowSignedNum(3 , 1 , ay , 4);
  OLED_ShowSignedNum(3 , 8 , gy , 4);
  OLED_ShowSignedNum(4 , 1 , az , 4);
  OLED_ShowSignedNum(4 , 8 , gz , 4);
}
