#include "stm32f10x.h"                  // Device header
#include "FM.h"
#include "Delay.h"
#include "GM.h"
#include "LED.h"

int main(void) {
	FM_Init();
	GM_Init();
	LED_Init();
	while(1) {
		if (GM_Get_Number() == 0) {
			FM_ON();  
			LED_ON();
		}
		else {
			FM_OFF();
			LED_OFF();
		}
	}
}
