#include "APP_MENU.h"

// ===================== 内部私有定义 =====================
// 菜单栈元素（用于保存历史路径）
typedef struct {
  const Menu_Item_t *parent; // 当时的父节点
  uint8_t cursor; // 当时的光标位置
} Menu_Stack_Item_t;

// ===================== 内部静态变量 =====================
static Menu_Stack_Item_t s_menu_stack[MENU_MAX_STACK_DEPTH]; // 菜单栈
static uint8_t s_stack_top = 0; // 栈顶指针（0表示栈空，只有根菜单）

static const Menu_Item_t *s_current_parent = NULL; // 当前父节点
static uint8_t s_current_cursor = 0; // 当前光标

// ===================== 内部栈操作函数（仅内部使用） =====================
// 入栈：保存当前状态
static void Menu_Stack_Push(void) {
  if (s_stack_top < MENU_MAX_STACK_DEPTH - 1) {
    s_menu_stack[s_stack_top].parent = s_current_parent;
    s_menu_stack[s_stack_top].cursor = s_current_cursor;
    s_stack_top++;
  }
}

// 出栈：恢复上一级状态
static uint8_t Menu_Stack_Pop(void) {
  if (s_stack_top > 0) {
    s_stack_top--;
    s_current_parent = s_menu_stack[s_stack_top].parent;
    s_current_cursor = s_menu_stack[s_stack_top].cursor;
    return 1; // 出栈成功
  }
  return 0; // 已经是根菜单，无法出栈
}

// 清空栈：回到根菜单
static void Menu_Stack_Clear(void) {
  s_stack_top = 0;
}

// ===================== 内部菜单逻辑函数（仅内部使用） =====================
// 处理“确认/进入”按键
static void Menu_Handle_Enter(void) {
  const Menu_Item_t *current_item = &s_current_parent[s_current_cursor];

  // 优先级1：如果有子菜单（是文件夹），进入子菜单
  if (current_item->children != NULL && current_item->child_count > 0) {
    Menu_Stack_Push(); // 保存当前状态
    s_current_parent = current_item->children; // 切换到子菜单
    s_current_cursor = 0; // 光标归零
  }
  // 优先级2：如果有执行函数（是文件），执行函数
  else if (current_item->action != NULL) {
    current_item->action();
  }
}

// 处理“下”按键
static void Menu_Handle_Down(void) {
  if (s_current_cursor < s_current_parent->child_count - 1) {
    s_current_cursor++;
  } else {
    // 可选：循环到顶部
    // s_current_cursor = 0;
  }
}

// 处理“上/返回”按键
static void Menu_Handle_Up_Back(void) {
  // 尝试出栈（返回上一级）
  // 如果出栈失败（已经是根菜单），则只移动光标
  if (Menu_Stack_Pop() == 0) {
    // 已经是根菜单，只移动光标
    if (s_current_cursor > 0) {
      s_current_cursor--;
    } else {
      // 可选：循环到底部
      // s_current_cursor = s_current_parent->child_count - 1;
    }
  }
}

// ===================== 对外接口函数实现 =====================
void App_Menu_Init(const Menu_Item_t *root_menu) {
  // 初始化菜单栈
  Menu_Stack_Clear();

  // 设置根菜单
  // 注意：根菜单是一个虚拟的父节点，它的children就是我们传入的根菜单数组
  // 这里我们用一个小技巧：直接把传入的数组当作父节点
  s_current_parent = root_menu;
  s_current_cursor = 0;
}

void App_Menu_Input_Key(Key_ID_t id, Key_Event_t evt) {
  // 我们只响应短按事件，长按可以在main.c里单独处理（比如长按返回主页）
  if (evt != KEY_EVT_SHORT_PRESS) {
    return;
  }

  // 根据你的按键映射处理逻辑
  // 假设：
  // KEY1 = 确认/进入
  // KEY2 = 下
  // KEY3 = 上/返回
  switch (id) {
    case KEY_ID_1:
      Menu_Handle_Enter();
      break;
    case KEY_ID_2:
      Menu_Handle_Down();
      break;
    case KEY_ID_3:
      Menu_Handle_Up_Back();
      break;
    default:
      break;
  }
}

uint8_t App_Menu_Get_Cursor(void) {
  return s_current_cursor;
}

const Menu_Item_t *App_Menu_Get_Current_Parent(void) {
  return s_current_parent;
}

void App_Menu_Go_Back(void) {
  Menu_Stack_Pop();
}

void App_Menu_Jump_To(const Menu_Item_t *menu) {
  Menu_Stack_Clear();
  s_current_parent = menu;
  s_current_cursor = 0;
}
