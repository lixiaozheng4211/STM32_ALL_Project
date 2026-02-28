#include "encoder.h"

// ================= 静态变量（内部使用） =================
static uint8_t Encoder_A_Last = 1;        // A相上一次状态（HAL库默认高电平）
static uint8_t Encoder_Key_Last = 1;      // 按键上一次状态
static uint16_t Encoder_Key_Cnt = 0;      // 按键长按计数（10ms/次）
static Encoder_Event_t Encoder_Event = ENCODER_NONE; // 事件缓存

// ================= TIM2中断回调（10ms执行一次） =================
void Encoder_TIM2_Callback(void)
{
    // 1. 读取当前引脚状态（HAL库接口）
    uint8_t a_now = HAL_GPIO_ReadPin(ENCODER_A_GPIO_PORT, ENCODER_A_GPIO_PIN);
    uint8_t b_now = HAL_GPIO_ReadPin(ENCODER_B_GPIO_PORT, ENCODER_B_GPIO_PIN);
    uint8_t key_now = HAL_GPIO_ReadPin(ENCODER_KEY_GPIO_PORT, ENCODER_KEY_GPIO_PIN);

    // 2. 扫描旋转事件（仅A相跳变时检测B相）
    if(a_now != Encoder_A_Last)
    {
        Encoder_A_Last = a_now;
        if(a_now == GPIO_PIN_SET && b_now == GPIO_PIN_RESET)
        {
            Encoder_Event = ENCODER_CW;   // 顺时针
        }
        else if(a_now == GPIO_PIN_SET && b_now == GPIO_PIN_SET)
        {
            Encoder_Event = ENCODER_CCW;  // 逆时针
        }
    }

    // 3. 扫描按键事件（消抖+长按检测）
    if(key_now != Encoder_Key_Last)
    {
        Encoder_Key_Last = key_now;
        Encoder_Key_Cnt = 0; // 状态变化，重置计数
    }
    else
    {
        if(key_now == GPIO_PIN_RESET) // 按键按下（上拉输入，低电平有效）
        {
            Encoder_Key_Cnt++;
            if(Encoder_Key_Cnt == 1) // 短按（10ms稳定后）
            {
                Encoder_Event = ENCODER_PRESS;
            }
            else if(Encoder_Key_Cnt >= 50) // 长按（50*10ms=500ms）
            {
                Encoder_Event = ENCODER_LONG_PRESS;
                Encoder_Key_Cnt = 0; // 防止重复触发
            }
        }
    }
}

// ================= 编码器初始化 =================
void Encoder_Init(void)
{
    // 引脚初始化已由CubeMX生成（MX_GPIO_Init()），这里仅初始化状态
    Encoder_A_Last = HAL_GPIO_ReadPin(ENCODER_A_GPIO_PORT, ENCODER_A_GPIO_PIN);
    Encoder_Key_Last = HAL_GPIO_ReadPin(ENCODER_KEY_GPIO_PORT, ENCODER_KEY_GPIO_PIN);
    Encoder_Event = ENCODER_NONE;
}

// ================= 获取编码器事件（外部调用） =================
Encoder_Event_t Encoder_Get_Event(void)
{
    Encoder_Event_t event = Encoder_Event;
    Encoder_Event = ENCODER_NONE; // 读取后清空事件
    return event;
}