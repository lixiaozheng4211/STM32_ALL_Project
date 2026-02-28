#include "app.h"
#include "Menu_Types.h"
#include "Page_Main.h"

#define DISPLAY_MAX_LINES 3


static MenuPage_t *CurrentPage = &MainPage; // 引擎核心指针
static uint8_t page_init_flag = 1; // 页面刷新标志
static int8_t last_cursor_pos = 0; // 用于通用列表的差分刷新

void App_Init(void) {
  OLED_Init();
  BSP_Encoder_Init();
  Menu_System_Setup();
}

void App_Run(void) {
  uint32_t scan_timer = HAL_GetTick();
  uint32_t UltaSound_Timer = HAL_GetTick();
  while (1) {
    if (HAL_GetTick() - scan_timer >= 10) {
      scan_timer = HAL_GetTick();
      Encoder_Event_t event = BSP_Encoder_Scan();
      // 1. 全局长按返回逻辑
      if (event == ENC_EVENT_LONG_PRESS && CurrentPage->Parent != NULL) {
        OLED_Clear();
        CurrentPage = CurrentPage->Parent;
        page_init_flag = 1;
        event = ENC_EVENT_NONE;
      }

      if (event != ENC_EVENT_NONE) {
        if (CurrentPage->CustomLogic != NULL) {
          CurrentPage->CustomLogic(event);
        } else {
          // B. 通用列表页处理 (滚屏与跳转)
          // 1. 光标移动
          if (event == ENC_EVENT_DOWN && CurrentPage->CursorPos < CurrentPage->ItemCount - 1) {
            CurrentPage->CursorPos++;
          } else if (event == ENC_EVENT_UP && CurrentPage->CursorPos > 0) {
            CurrentPage->CursorPos--;
          }
          // 2. 滑动窗口拉扯
          if (CurrentPage->CursorPos < CurrentPage->ScrollOffset) {
            CurrentPage->ScrollOffset = CurrentPage->CursorPos;
            page_init_flag = 1;
          } else if (CurrentPage->CursorPos >= CurrentPage->ScrollOffset + DISPLAY_MAX_LINES) {
            CurrentPage->ScrollOffset = CurrentPage->CursorPos - DISPLAY_MAX_LINES + 1;
            page_init_flag = 1;
          }
          // 3. 点击进入子节点
          if (event == ENC_EVENT_SHORT_PRESS && CurrentPage->ItemCount > 0) {
            MenuItem_t *sel = &CurrentPage->Items[CurrentPage->CursorPos];
            if (sel->ChildPage != NULL) {
              OLED_Clear();
              CurrentPage = sel->ChildPage; // 树指针下沉！
              page_init_flag = 1;
            } else if (sel->ActionFunc != NULL) {
              sel->ActionFunc(); // 执行绑定的函数
            }
          }
        }
      }

      if (CurrentPage->CustomRender != NULL) {
        // A. 渲染自定义页面
        CurrentPage->CustomRender(page_init_flag);
        page_init_flag = 0;
      } else {
        // B. 渲染通用列表页 (带滚屏和差分刷新)
        if (page_init_flag) {
          OLED_ShowString(1, 1, (char *) CurrentPage->PageTitle);
          // 遍历滑动窗口内的项
          for (uint8_t i = 0; i < DISPLAY_MAX_LINES && (i + CurrentPage->ScrollOffset) < CurrentPage->ItemCount; i++) {
            OLED_ShowString(i + 2, 2, (char *) CurrentPage->Items[i + CurrentPage->ScrollOffset].ItemName);
          }
          // 画光标
          int8_t rel_row = CurrentPage->CursorPos - CurrentPage->ScrollOffset;
          OLED_ShowChar(rel_row + 2, 1, '>');

          last_cursor_pos = CurrentPage->CursorPos;
          page_init_flag = 0;
        } else if (last_cursor_pos != CurrentPage->CursorPos) {
          // 差分刷新光标
          int8_t old_rel = last_cursor_pos - CurrentPage->ScrollOffset;
          int8_t new_rel = CurrentPage->CursorPos - CurrentPage->ScrollOffset;

          OLED_ShowChar(old_rel + 2, 1, ' ');
          OLED_ShowChar(new_rel + 2, 1, '>');
          last_cursor_pos = CurrentPage->CursorPos;
        }
      }
    }
    if (HAL_GetTick() - UltaSound_Timer >= 100) {
      UltaSound_Timer = HAL_GetTick();
      BSP_Sonar_Trig();
    }
  }
}
