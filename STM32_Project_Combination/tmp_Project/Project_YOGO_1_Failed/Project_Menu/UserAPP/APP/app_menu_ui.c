#include "app_menu_ui.h"
#include "app_menu.h"
#include "OLED.h"
#include <string.h>

#define VISIBLE_LINES   3   /* 第2~4行显示菜单项 */
#define LINE_WIDTH      16  /* OLED每行字符数 */

static uint8_t window_start = 0;
static uint8_t needsRedraw = 1;

static void show_padded(uint8_t line, uint8_t col, const char *str, uint8_t max_len)
{
    char buf[17];
    uint8_t len = strlen(str);
    if (len > max_len) len = max_len;
    memcpy(buf, str, len);
    memset(buf + len, ' ', max_len - len);
    buf[max_len] = '\0';
    OLED_ShowString(line, col, buf);
}

void App_Menu_UI_SetRedraw(void)
{
    needsRedraw = 1;
}

void App_Menu_UI_Draw(void)
{
    if (!needsRedraw)
        return;
    needsRedraw = 0;

    uint8_t cursor = App_Menu_GetCursor();
    const Menu_Item_t *menu = App_Menu_GetCurrentMenu();
    uint8_t count = App_Menu_GetCurrentCount();
    const char *title = App_Menu_GetTitle();

    if (menu == NULL)
        return;

    /* 滚动窗口计算 */
    if (cursor < window_start)
        window_start = cursor;
    if (cursor >= window_start + VISIBLE_LINES)
        window_start = cursor - VISIBLE_LINES + 1;
    if (window_start + VISIBLE_LINES > count && count >= VISIBLE_LINES)
        window_start = count - VISIBLE_LINES;

    /* 第1行：标题 */
    show_padded(1, 1, title, LINE_WIDTH);

    /* 第2~4行：菜单项 */
    for (uint8_t i = 0; i < VISIBLE_LINES; i++) {
        uint8_t idx = window_start + i;
        uint8_t line = i + 2;

        if (idx < count) {
            char prefix = (idx == cursor) ? '>' : ' ';
            OLED_ShowChar(line, 1, prefix);
            show_padded(line, 2, menu[idx].name, LINE_WIDTH - 1);
        } else {
            show_padded(line, 1, "", LINE_WIDTH);
        }
    }
}
