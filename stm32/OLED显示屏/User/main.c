#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "cstring"

void OLED_Show_Arr(uint8_t * arr) {
	uint8_t ArrLength = strlen((char*)arr);
	if (ArrLength > 64) {
		OLED_ShowString(1 , 1 , "too long !!!"); 
		return;
	}
	uint32_t CurRow = (uint32_t)1 ,CurCol = (uint32_t)1;
	for (uint32_t i = 0 ; i < ArrLength ; i++) {
		OLED_ShowChar(CurRow , CurCol , arr[i]);
		CurCol++;
		if (CurCol > 16) {
			CurRow++;
			CurCol = 1;
		}
	}
	return;
}


int main(void) {
	OLED_Init();
	uint8_t arr[] = "-1 2 3 4 -5 6 7 8 9 -10 11 -12";
	while(1) OLED_Show_Arr(arr);
}
