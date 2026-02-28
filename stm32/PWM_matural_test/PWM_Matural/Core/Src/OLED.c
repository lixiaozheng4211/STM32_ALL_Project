#include "../Inc/OLED.h"
#include "oled_font.h"
#include "stdio.h"

/**
  * @brief  I2C开始信号
  */
void OLED_I2C_Start(void)
{
    // 在HAL库的硬件I2C中，开始信号由硬件自动处理
    // 此函数在硬件I2C模式下可以留空或删除
}

/**
  * @brief  I2C停止信号
  */
void OLED_I2C_Stop(void)
{
    // 在HAL库的硬件I2C中，停止信号由硬件自动处理
    // 此函数在硬件I2C模式下可以留空或删除
}

/**
  * @brief  使用HAL库的I2C发送数据
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
    // 这个函数在硬件I2C模式下不再需要
    // 数据发送由HAL_I2C_Mem_Write处理
}

/**
  * @brief  OLED写命令 - 使用HAL库硬件I2C
  */
void OLED_WriteCommand(uint8_t Command)
{
    uint8_t cmd_buffer[2];
    cmd_buffer[0] = OLED_CMD;  // 控制字节：写命令
    cmd_buffer[1] = Command;    // 命令数据
    
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, cmd_buffer, 2, HAL_MAX_DELAY);
}

/**
  * @brief  OLED写数据 - 使用HAL库硬件I2C
  */
void OLED_WriteData(uint8_t Data)
{
    uint8_t data_buffer[2];
    data_buffer[0] = OLED_DATA;  // 控制字节：写数据
    data_buffer[1] = Data;       // 显示数据
    
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, data_buffer, 2, HAL_MAX_DELAY);
}

/**
  * @brief  替代方案：使用Mem_Write函数（更简洁）
  */
void OLED_WriteCommand_Alt(uint8_t Command)
{
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDRESS, OLED_CMD, I2C_MEMADD_SIZE_8BIT, &Command, 1, HAL_MAX_DELAY);
}

void OLED_WriteData_Alt(uint8_t Data)
{
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDRESS, OLED_DATA, I2C_MEMADD_SIZE_8BIT, &Data, 1, HAL_MAX_DELAY);
}

/**
  * @brief  OLED设置光标位置
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);                    // 设置Y位置
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));   // 设置X位置高4位
    OLED_WriteCommand(0x00 | (X & 0x0F));           // 设置X位置低4位
}

/**
  * @brief  OLED清屏
  */
void OLED_Clear(void)
{  
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for(i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

/**
  * @brief  OLED显示一个字符
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);       // 设置光标位置在上半部分
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);          // 显示上半部分内容
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);   // 设置光标位置在下半部分
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);      // 显示下半部分内容
    }
}

/**
  * @brief  OLED显示字符串
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

/**
  * @brief  次方计算函数
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)							
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;
    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }
    for (i = 0; i < Length; i++)							
    {
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)							
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
        {
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        }
        else
        {
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
        }
    }
}

/**
  * @brief  OLED初始化
  */
void OLED_Init(void)
{
    HAL_Delay(100);  // 上电延时，等待OLED稳定
    
    // 注意：GPIO初始化现在应该在CubeMX中配置
    // 或者使用MX_I2C1_Init()函数初始化
    
    OLED_WriteCommand(0xAE);    // 关闭显示
    
    OLED_WriteCommand(0xD5);    // 设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);
    
    OLED_WriteCommand(0xA8);    // 设置多路复用率
    OLED_WriteCommand(0x3F);
    
    OLED_WriteCommand(0xD3);    // 设置显示偏移
    OLED_WriteCommand(0x00);
    
    OLED_WriteCommand(0x40);    // 设置显示开始行
    
    OLED_WriteCommand(0xA1);    // 设置左右方向，0xA1正常 0xA0左右反置
    
    OLED_WriteCommand(0xC8);    // 设置上下方向，0xC8正常 0xC0上下反置

    OLED_WriteCommand(0xDA);    // 设置COM引脚硬件配置
    OLED_WriteCommand(0x12);
    
    OLED_WriteCommand(0x81);    // 设置对比度控制
    OLED_WriteCommand(0xCF);

    OLED_WriteCommand(0xD9);    // 设置预充电周期
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB);    // 设置VCOMH取消选择级别
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4);    // 设置整个显示打开/关闭

    OLED_WriteCommand(0xA6);    // 设置正常/倒转显示

    OLED_WriteCommand(0x8D);    // 设置充电泵
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF);    // 开启显示
        
    OLED_Clear();              // OLED清屏
}