#include "Page_Main.h"
#include "OLED.h"
#include "BSP_UltraSound.h"


extern MenuPage_t MainPage; // 主菜单页面 , root房间
extern MenuPage_t Muti_ADC; // ADC 的门


// ================================= ADC =============================================
static void Light_Sensor_Render(uint8_t is_init) {
  if (is_init) {
    OLED_Clear();
    OLED_ShowString(1, 1, "- Light Sensor -");
    OLED_ShowString(2, 1, "Data: ");
    OLED_ShowString(4, 1, "LongPress Exit");
  }
}

MenuPage_t Light_Sensor = {
  .PageTitle = "Light_Sensor",
  .Items = NULL,
  .Parent = &Muti_ADC,
  .ItemCount = 0,
  .ScrollOffset = 0,
  .CursorPos = 0,
  .CustomLogic = NULL,
  .CustomRender = Light_Sensor_Render, // 光敏传感器只有一个渲染 , 其他没有
};

static void Infrared_sensor_Render(uint8_t is_init) {
  if (is_init) {
    OLED_Clear();
    OLED_ShowString(1, 1, "-Infrared_sensor-");
    OLED_ShowString(2, 1, "Data: ");
    OLED_ShowString(4, 1, "LongPress Exit");
  }
}

MenuPage_t Infrared_sensor = {
  .PageTitle = "Infrared_sensor",
  .Items = NULL,
  .Parent = &Muti_ADC,
  .ItemCount = 0,
  .ScrollOffset = 0,
  .CursorPos = 0,
  .CustomLogic = NULL,
  .CustomRender = Infrared_sensor_Render, // 红外传感器只有一个渲染 , 其他没有
};

static MenuItem_t ADC_Items[] = {
  {.ChildPage = &Light_Sensor, .ActionFunc = NULL, .ItemName = "Light_Sensor"},
  {.ChildPage = &Infrared_sensor, .ActionFunc = NULL, .ItemName = "Infrared_sensor"},
};

MenuPage_t Muti_ADC = {
  .PageTitle = "Muti ADC       ",
  .Items = ADC_Items,
  .ItemCount = 2,
  .CursorPos = 0,
  .ScrollOffset = 0,
  .Parent = &MainPage,
  .CustomLogic = NULL,
  .CustomRender = NULL,
};
// ================================= ADC =============================================


// ================================= UltraSound =======================================
static void UltraSound_Render(uint8_t is_init) {
  // is_init == 1：代表刚进房间的第一帧
  if (is_init) {
    OLED_Clear(); // 进门先大清屏
    OLED_ShowString(1, 1, "- Ultra Sound -");
    OLED_ShowString(2, 1, "Dist: ");
    OLED_ShowString(4, 1, "LongPress Exit"); // 提示用户怎么退出去
  }
  if (System_Distance_cm > 900.0f) {
    OLED_ShowString(2, 7, "ERROR   ");
  } else {
    OLED_ShowNum(2, 7, (uint32_t) System_Distance_cm, 3);
    OLED_ShowString(2, 10, " cm  ");
  }
}

MenuPage_t Ultra_Sound = {
  .PageTitle = "Ultra Sound",
  .ItemCount = 0,
  .Items = NULL,
  .Parent = &MainPage,
  .CursorPos = 0,
  .ScrollOffset = 0,
  .CustomLogic = NULL,
  .CustomRender = UltraSound_Render
};
// ================================= UltraSound =======================================


// 组装主菜单的门
static MenuItem_t MainItems[] = {
  // 门牌号
  {"1. Set Voltage ", NULL, NULL},
  {"2. System Info ", NULL, NULL},
  {"3. Ultra Sound ", &Ultra_Sound, NULL},
  {"4. MutiRoad ADC", &Muti_ADC, NULL}
};

MenuPage_t MainPage = {
  "--- MAIN MENU ---", 4, MainItems, NULL, 0, 0, NULL, NULL
};

// 3. 全局连线网管函数 (开机时调用一次)
void Menu_System_Setup(void) {
  Ultra_Sound.Parent = &MainPage;
}
