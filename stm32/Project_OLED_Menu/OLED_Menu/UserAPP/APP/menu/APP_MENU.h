#ifndef __APP_MENU_H
#define __APP_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <string.h>
  // 如果需要用到按键枚举，包含BSP层头文件（仅用枚举，不调用硬件函数）
#include "bsp_key.h"

  // ===================== 配置区 =====================
#define MENU_MAX_STACK_DEPTH  8  // 菜单栈深度（支持最多8级嵌套）
#define MENU_NAME_MAX_LEN     16 // 菜单名称最大长度

  // ===================== 核心菜单节点结构体 =====================
  // 前向声明
  typedef struct Menu_Item Menu_Item_t;

  // 菜单节点结构体
  struct Menu_Item {
    const char *name;             // 菜单名称（字符串常量）
    void (*action)(void);         // 功能函数指针（如果是可执行项，填函数；如果是文件夹，填NULL）
    const Menu_Item_t *children;  // 子菜单数组指针（如果是文件夹，填子菜单数组首地址；如果是可执行项，填NULL）
    uint8_t child_count;          // 子菜单数量（children数组的元素个数）
  };

  // ===================== 对外接口函数声明 =====================
  /**
   * @brief  菜单初始化
   * @param  root_menu 根菜单数组的首地址
   * @retval 无
   */
  void App_Menu_Init(const Menu_Item_t *root_menu);

  /**
   * @brief  输入按键事件（由main.c的按键回调调用）
   * @param  id 按键ID
   * @param  evt 按键事件
   * @retval 无
   */
  void App_Menu_Input_Key(Key_ID_t id, Key_Event_t evt);

  /**
   * @brief  获取当前菜单的光标位置（供显示层调用）
   * @param  无
   * @retval 当前光标索引（0开始）
   */
  uint8_t App_Menu_Get_Cursor(void);

  /**
   * @brief  获取当前菜单的父节点（供显示层调用，用于遍历当前页的所有菜单项）
   * @param  无
   * @retval 当前父节点的指针
   */
  const Menu_Item_t* App_Menu_Get_Current_Parent(void);

  /**
   * @brief  手动返回到上一级菜单（供功能函数调用，比如“设置完成后自动返回”）
   * @param  无
   * @retval 无
   */
  void App_Menu_Go_Back(void);

  /**
   * @brief  手动跳转到指定菜单（供功能函数调用，比如“长按返回主页”）
   * @param  menu 要跳转到的菜单指针
   * @retval 无
   */
  void App_Menu_Jump_To(const Menu_Item_t *menu);

#ifdef __cplusplus
}
#endif

#endif // __APP_MENU_H