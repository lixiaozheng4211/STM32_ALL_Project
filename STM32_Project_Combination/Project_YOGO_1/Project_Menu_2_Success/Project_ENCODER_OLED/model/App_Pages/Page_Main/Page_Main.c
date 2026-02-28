#include "Page_Main.h"


// 1. 组装主菜单的门
static MenuItem_t MainItems[] = {
  // 门牌号
  {"1. Set Voltage",  NULL,          NULL},
  {"2. System Info",  NULL,          NULL},
  {"3. KO",     NULL,  NULL},
  {"4. KO" , NULL , NULL}
};

// 2. 组装主菜单大厅
MenuPage_t MainPage = {
  "--- MAIN MENU ---", 4, MainItems, NULL, 0, 0, NULL, NULL
};

// 3. 全局连线网管函数 (开机时调用一次)
void Menu_System_Setup(void) {

}