#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Encoder.h"

int main(void) {
	OLED_Init();
	Encoder_Init();
	OLED_ShowString(1 , 1,"Count:");
	while(1) {
		int32_t Num = Get_Encoder_Count();
		OLED_ShowSignedNum(1,7,Num,5);
	}
}
