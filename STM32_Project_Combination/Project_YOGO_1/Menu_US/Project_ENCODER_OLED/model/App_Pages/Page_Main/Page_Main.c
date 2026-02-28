#include "Page_Main.h"
#include "OLED.h"
#include "BSP_UltraSound.h"


extern MenuPage_t MainPage; // 主菜单页面 , root房间


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


// 1. 组装主菜单的门
static MenuItem_t MainItems[] = {
  // 门牌号
  {"1. Set Voltage ",  NULL,          NULL},
  {"2. System Info ",  NULL,          NULL},
  {"3. Ultra Sound ",     &Ultra_Sound,  NULL},
  {"4. About US    " , NULL , NULL}
};

// 2. 组装主菜单大厅
MenuPage_t MainPage = {
  "--- MAIN MENU ---", 4, MainItems, NULL, 0, 0, NULL, NULL
};

// 3. 全局连线网管函数 (开机时调用一次)
void Menu_System_Setup(void) {
  Ultra_Sound.Parent = &MainPage;
}