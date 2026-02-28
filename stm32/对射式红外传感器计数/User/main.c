#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "hwsensor.h"
#include "LED.h"

int main(void) {
	//LED_Init();
	OLED_Init();
	//Hw_Sensor_Init();
	OLED_ShowString(1 , 1,"Count:");
	while(1) {
		//OLED_ShowNum(1 , 7 , Count_Sensor_GET() , 5);
	}
}
