#ifndef __MENU_TYPES_H
#define __MENU_TYPES_H

#include "main.h"
#include "bsp_encoder.h"

typedef struct MenuPage MenuPage_t;

/* 菜单项结构体 */
typedef struct {
  const char *ItemName;      // 菜单项显示名称
  MenuPage_t *ChildPage;     // 指向子菜单页的指针（若无可为NULL）
  void (*ActionFunc)(void);  // 菜单项触发时的回调函数指针
} MenuItem_t;

/* 菜单页结构体 */
struct MenuPage {
  const char *PageTitle;     // 当前页面的标题文字
  uint8_t ItemCount;         // 当前页面包含的菜单项数量
  MenuItem_t *Items;         // 菜单项数组指针
  MenuPage_t *Parent;        // 指向父级菜单页的指针（用于返回上一级）
  int8_t CursorPos;          // 当前光标选中的菜单项索引（0 ~ ItemCount-1）
  int8_t ScrollOffset;       // 当前显示时的滚动偏移量（用于分页显示）
  void (*CustomRender)(uint8_t is_init);  // 自定义页面渲染函数
  void (*CustomLogic)(Encoder_Event_t event);  // 自定义逻辑处理函数（通常处理编码器事件）
};

extern MenuPage_t MainPage;
#endif