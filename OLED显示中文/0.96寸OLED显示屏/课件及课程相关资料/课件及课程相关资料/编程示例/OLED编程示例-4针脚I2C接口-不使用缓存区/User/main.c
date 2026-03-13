#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"


int main(void)
{
	OLED_Init();
	OLED_Clear();
	OLED_ShowChinese(0, 0, "你好世界");
	OLED_ShowChinese(0, 2, "你好世界");
	
	while (1)
	{
		
	}
}
