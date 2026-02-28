#include "app_menu.h"
#include "app_menu_ui.h"

/* ---- 导航栈 ---- */
#define NAV_STACK_DEPTH  8

typedef struct {
    const Menu_Item_t *menu;
    uint8_t count;
    uint8_t cursor;
    const char *title;
} Nav_Level_t;

static Nav_Level_t nav_stack[NAV_STACK_DEPTH];
static int8_t nav_top = -1;

/* ---- 当前状态 ---- */
static const Menu_Item_t *cur_menu = NULL;
static uint8_t cur_count = 0;
static uint8_t cur_cursor = 0;
static const char *cur_title = "Menu";

/* ---- 公开接口 ---- */
void App_Menu_Init(const Menu_Item_t *root, uint8_t root_count)
{
    cur_menu = root;
    cur_count = root_count;
    cur_cursor = 0;
    cur_title = "Menu";
    nav_top = -1;
    App_Menu_UI_SetRedraw();
}

void App_Menu_Process(Encoder_Event_t evt)
{
    if (evt == ENC_EVT_NONE || cur_menu == NULL)
        return;

    switch (evt) {
    case ENC_EVT_CW:
        if (cur_count > 0) {
            cur_cursor = (cur_cursor + 1) % cur_count;
            App_Menu_UI_SetRedraw();
        }
        break;

    case ENC_EVT_CCW:
        if (cur_count > 0) {
            cur_cursor = (cur_cursor + cur_count - 1) % cur_count;
            App_Menu_UI_SetRedraw();
        }
        break;

    case ENC_EVT_SHORT_PRESS: {
        const Menu_Item_t *item = &cur_menu[cur_cursor];
        if (item->children != NULL && item->child_count > 0) {
            /* 压栈，进入子菜单 */
            if (nav_top < NAV_STACK_DEPTH - 1) {
                nav_top++;
                nav_stack[nav_top].menu = cur_menu;
                nav_stack[nav_top].count = cur_count;
                nav_stack[nav_top].cursor = cur_cursor;
                nav_stack[nav_top].title = cur_title;

                cur_title = item->name;
                cur_menu = item->children;
                cur_count = item->child_count;
                cur_cursor = 0;
                App_Menu_UI_SetRedraw();
            }
        } else if (item->action != NULL) {
            item->action();
            App_Menu_UI_SetRedraw();
        }
        break;
    }

    case ENC_EVT_LONG_PRESS:
        /* 返回上级 */
        if (nav_top >= 0) {
            cur_menu = nav_stack[nav_top].menu;
            cur_count = nav_stack[nav_top].count;
            cur_cursor = nav_stack[nav_top].cursor;
            cur_title = nav_stack[nav_top].title;
            nav_top--;
            App_Menu_UI_SetRedraw();
        }
        break;

    default:
        break;
    }
}

uint8_t App_Menu_GetCursor(void)
{
    return cur_cursor;
}

const Menu_Item_t *App_Menu_GetCurrentMenu(void)
{
    return cur_menu;
}

uint8_t App_Menu_GetCurrentCount(void)
{
    return cur_count;
}

const char *App_Menu_GetTitle(void)
{
    return cur_title;
}
