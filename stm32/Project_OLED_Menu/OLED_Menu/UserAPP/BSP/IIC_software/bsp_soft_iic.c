#include "bsp_soft_iic.h"


// 软件微秒级延时（72MHz主频适配，通讯不稳定就调大SOFT_I2C_DELAY_US）
static void Soft_I2C_Delay(uint32_t us) {
  uint32_t i;
  for (i = 0; i < us * 1; i++) {
    __NOP(); // 空指令，保证延时稳定
  }
}

// ===================== 对外接口函数实现 =====================
/**
 * @brief  软件I2C引脚初始化
 * @param
 * @retval
 */
void Soft_I2C_Init(void) {
  // 软件模拟IIC其实就是开启GPIO口
  // GPIO时钟使能：必须开启对应端口的时钟，否则引脚不工作
  __HAL_RCC_GPIOB_CLK_ENABLE(); // 你的SCL/SDA都在PB口，开启GPIOB时钟

  // 初始化SCL引脚：推挽输出，空闲电平高
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = SOFT_I2C_SCL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SOFT_I2C_SCL_PORT, &GPIO_InitStruct);

  // 初始化SDA引脚：默认推挽输出，空闲电平高
  GPIO_InitStruct.Pin = SOFT_I2C_SDA_PIN;
  HAL_GPIO_Init(SOFT_I2C_SDA_PORT, &GPIO_InitStruct);

  // I2C总线空闲状态：SCL和SDA都为高电平
  SOFT_I2C_SCL_H();
  SOFT_I2C_SDA_H();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);
}

/**
 * @brief  产生I2C起始信号
 * @param
 * @retval
 */
void Soft_I2C_Start(void) {
  SOFT_I2C_SDA_OUTPUT(); // SDA设为输出模式
  SOFT_I2C_SDA_H();
  SOFT_I2C_SCL_H();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);

  // 起始信号：SCL为高时，SDA从高变低
  SOFT_I2C_SDA_L();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);

  // 钳住SCL，准备发送数据
  SOFT_I2C_SCL_L();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);
}

/**
 * @brief  产生I2C停止信号
 * @param
 * @retval
 */
void Soft_I2C_Stop(void) {
  SOFT_I2C_SDA_OUTPUT(); // SDA设为输出模式
  SOFT_I2C_SCL_L();
  SOFT_I2C_SDA_L();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);

  // 停止信号：SCL为高时，SDA从低变高
  SOFT_I2C_SCL_H();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);
  SOFT_I2C_SDA_H();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);
}

/**
 * @brief  等待从机应答信号
 * @param
 * @retval 0=应答成功，1=应答失败/超时
 */
uint8_t Soft_I2C_Wait_Ack(void) {
  uint8_t timeout = 0;

  SOFT_I2C_SDA_INPUT(); // SDA设为输入模式，读取从机电平
  SOFT_I2C_SDA_H(); // 释放SDA总线
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);

  SOFT_I2C_SCL_H(); // SCL拉高，读取应答
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);

  // 等待从机拉低SDA（应答信号）如果是1就要进入循环 , 如果是0就可以直接退出循环 -> 非常巧妙
  while (SOFT_I2C_SDA_READ()) {
    timeout++;
    if (timeout > SOFT_I2C_ACK_TIMEOUT) {
      Soft_I2C_Stop(); // 超时，产生停止信号
      return 1; // 应答失败
    }
  }

  SOFT_I2C_SCL_L(); // 拉低SCL，结束应答
  return 0; // 应答成功
}

/**
 * @brief  主机发送应答信号（继续读取下一个字节时用）
 * @param
 * @retval
 */
void Soft_I2C_Send_Ack(void) {
  SOFT_I2C_SCL_L();
  SOFT_I2C_SDA_OUTPUT();

  // 应答信号：SCL从低到高时，SDA保持低电平
  SOFT_I2C_SDA_L();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);

  SOFT_I2C_SCL_H();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);

  SOFT_I2C_SCL_L();
}

/**
 * @brief  主机发送非应答信号（结束读取时用）
 * @param
 * @retval
 */
void Soft_I2C_Send_NAck(void) {
  SOFT_I2C_SCL_L();
  SOFT_I2C_SDA_OUTPUT();

  // 非应答信号：SCL从低到高时，SDA保持高电平
  SOFT_I2C_SDA_H();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);

  SOFT_I2C_SCL_H();
  Soft_I2C_Delay(SOFT_I2C_DELAY_US);

  SOFT_I2C_SCL_L();
}

/**
 * @brief  主机写1个字节（MSB高位在前，符合I2C标准）
 * @param  data 要写入的字节数据
 * @retval
 */
void Soft_I2C_Write_Byte(uint8_t data) {
  uint8_t i;
  SOFT_I2C_SDA_OUTPUT();
  SOFT_I2C_SCL_L();

  // 循环发送8位数据，从高位到低位
  for (i = 0; i < 8; i++) {
    if (data & 0x80) SOFT_I2C_SDA_H();
    else  SOFT_I2C_SDA_L();

    data <<= 1; // 左移1位，准备下一位
    Soft_I2C_Delay(SOFT_I2C_DELAY_US);

    // 拉高SCL，让从机读取数据
    SOFT_I2C_SCL_H();
    Soft_I2C_Delay(SOFT_I2C_DELAY_US);

    // 拉低SCL，继续发送下一位
    SOFT_I2C_SCL_L();
    Soft_I2C_Delay(SOFT_I2C_DELAY_US);
  }
}

/**
 * @brief  主机读1个字节
 * @param  ack 1=读完发送应答（继续读），0=读完发送非应答（结束读）
 * @retval 读取到的字节数据
 */
uint8_t Soft_I2C_Read_Byte(uint8_t ack) {
  uint8_t i;
  uint8_t data = 0;

  SOFT_I2C_SDA_INPUT(); // SDA设为输入模式，接收数据

  // 循环读取8位数据，从高位到低位
  for (i = 0; i < 8; i++) {
    data <<= 1;
    SOFT_I2C_SCL_L();
    Soft_I2C_Delay(SOFT_I2C_DELAY_US);

    // 拉高SCL，读取SDA电平
    SOFT_I2C_SCL_H();
    Soft_I2C_Delay(SOFT_I2C_DELAY_US);

    if (SOFT_I2C_SDA_READ()) {
      data |= 0x01;
    }
  }

  // 发送应答/非应答
  if (ack) Soft_I2C_Send_Ack();
  else Soft_I2C_Send_NAck();
  return data;
}

// ===================== 上层通用封装函数 =====================
/**
 * @brief  写单个寄存器
 * @param  dev_addr 7位I2C设备地址（比如OLED的0x3C，不用左移）
 * @param  reg_addr 寄存器地址
 * @param  data 要写入的数据
 * @retval 0=成功，1=失败
 */
uint8_t Soft_I2C_Write_Reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data) {
  Soft_I2C_Start();
  // 发送设备地址+写标志（最低位0=写）
  Soft_I2C_Write_Byte((dev_addr << 1) | 0x00);
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  // 发送寄存器地址
  Soft_I2C_Write_Byte(reg_addr);
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  // 发送数据
  Soft_I2C_Write_Byte(data);
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  Soft_I2C_Stop();
  return 0;
}

/**
 * @brief  读单个寄存器
 * @param  dev_addr 7位I2C设备地址
 * @param  reg_addr 寄存器地址
 * @param  pdata 读取数据的存放地址
 * @retval 0=成功，1=失败
 */
uint8_t Soft_I2C_Read_Reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *pdata) {
  Soft_I2C_Start();
  // 发送设备地址+写标志，先写寄存器地址
  Soft_I2C_Write_Byte((dev_addr << 1) | 0x00);
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  Soft_I2C_Write_Byte(reg_addr);
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  // 重复起始信号，切换到读模式
  Soft_I2C_Start();
  Soft_I2C_Write_Byte((dev_addr << 1) | 0x01);
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  // 读取数据，发送非应答结束
  *pdata = Soft_I2C_Read_Byte(0);
  Soft_I2C_Stop();
  return 0;
}

/**
 * @brief  连续写多个字节
 * @param  dev_addr 7位I2C设备地址
 * @param  reg_addr 起始寄存器地址
 * @param  pbuf 数据缓冲区
 * @param  len 要写入的字节数
 * @retval 0=成功，1=失败
 */
uint8_t Soft_I2C_Write_Buffer(uint8_t dev_addr, uint8_t reg_addr, uint8_t *pbuf, uint16_t len) {
  uint16_t i;
  Soft_I2C_Start();
  Soft_I2C_Write_Byte((dev_addr << 1) | 0x00); // 后面这个 | 0x00是为了准确描述度操作
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  Soft_I2C_Write_Byte(reg_addr);
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  for (i = 0; i < len; i++) {
    Soft_I2C_Write_Byte(pbuf[i]);
    if (Soft_I2C_Wait_Ack() != 0) return 1;
  }

  Soft_I2C_Stop();
  return 0;
}

/**
 * @brief  连续读多个字节 , 指定地址读 ==> 标志 : 有一个restart
 * @param  dev_addr 7位I2C设备地址
 * @param  reg_addr 起始寄存器地址
 * @param  pbuf 数据缓冲区
 * @param  len 要读取的字节数
 * @retval 0=成功，1=失败
 */
uint8_t Soft_I2C_Read_Buffer(uint8_t dev_addr, uint8_t reg_addr, uint8_t *pbuf, uint16_t len) {
  uint16_t i;
  Soft_I2C_Start();
  Soft_I2C_Write_Byte((dev_addr << 1)|0x00);
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  Soft_I2C_Write_Byte(reg_addr);
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  Soft_I2C_Start();
  Soft_I2C_Write_Byte((dev_addr << 1) | 0x01);
  if (Soft_I2C_Wait_Ack() != 0) return 1;

  for (i = 0; i < len; i++) {
    // 最后一个字节发送非应答，前面的都发送应答
    if (i == len - 1) {
      pbuf[i] = Soft_I2C_Read_Byte(0);
    } else {
      pbuf[i] = Soft_I2C_Read_Byte(1);
    }
  }

  Soft_I2C_Stop();
  return 0;
}
