#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Motor.h"
#include "Delay.h"
#include "Key.h"

int main(void) {
	Motor_Init();
	OLED_Init();
	Key_Init();
	int8_t Motor_Speed = -100;
	int8_t Key_Num;
	OLED_ShowString(1,1,"MOTSPEED: ");
	while(1) {
		Key_Num = Key_GetNum();
		if (Key_Num == 1) {
			Set_Motor_Speed(Motor_Speed);
			OLED_ShowSignedNum(1,12,Motor_Speed,3);
			Motor_Speed += 10;
			if (Motor_Speed > 100) Motor_Speed = -100;
			
		}
	}
}
