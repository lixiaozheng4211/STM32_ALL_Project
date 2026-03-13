#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"

extern const uint8_t BMP_ICON[];

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	while (1)
	{	
		OLED_ShowCN(2,6,0,1);
		OLED_ShowCN(2,7,1,1);
		OLED_ShowCN(3,6,2,1);
		OLED_ShowCN(3,7,3,1);
		OLED_ShowCN(3,8,4,1);
		OLED_ShowBMP(0, 0, 64, 64, BMP_ICON,0);
		Delay_ms(1);
	}
}



