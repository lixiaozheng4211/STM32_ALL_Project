#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Servo.h"
#include "Delay.h"
#include "Key.h"

uint8_t Keynumber;
float angle;

int main(void) {
	OLED_Init();
	Servo_Init();
	OLED_ShowString(1,1,"angle:");
	while (1) {
		for (angle = 0 ; angle <= 180 ; angle++) {
			Set_ServoAngle(angle);
			Delay_ms(5);
			if (angle == 180) {
				Delay_ms(50);
			}
		}
		for (angle = 0 ; angle <= 180 ; angle++) {
			Set_ServoAngle(180 - angle);
			Delay_ms(5);
			if (angle == 0) {
				Delay_ms(50);
			}
		}
	}
}
