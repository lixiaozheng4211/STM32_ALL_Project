#ifndef __APP_MENU_H
#define __APP_MENU_H

#include <stdint.h>
#include "bsp_encoder.h"

typedef struct Menu_Item {
    const char *name;
    void (*action)(void);
    const struct Menu_Item *children;
    uint8_t child_count;
} Menu_Item_t;

void App_Menu_Init(const Menu_Item_t *root, uint8_t root_count);
void App_Menu_Process(Encoder_Event_t evt);
uint8_t App_Menu_GetCursor(void);
const Menu_Item_t *App_Menu_GetCurrentMenu(void);
uint8_t App_Menu_GetCurrentCount(void);
const char *App_Menu_GetTitle(void);

#endif
