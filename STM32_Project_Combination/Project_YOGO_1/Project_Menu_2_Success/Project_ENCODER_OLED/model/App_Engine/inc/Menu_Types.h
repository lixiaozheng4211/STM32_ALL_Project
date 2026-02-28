#ifndef __MENU_TYPES_H
#define __MENU_TYPES_H

#include "main.h"
#include "bsp_encoder.h"

typedef struct MenuPage MenuPage_t;

typedef struct {
  const char *ItemName;
  MenuPage_t *ChildPage;
  void (*ActionFunc)(void);
} MenuItem_t;

struct MenuPage {
  const char *PageTitle;
  uint8_t ItemCount;
  MenuItem_t *Items;
  MenuPage_t *Parent;
  int8_t CursorPos;
  int8_t ScrollOffset;
  void (*CustomRender)(uint8_t is_init);
  void (*CustomLogic)(Encoder_Event_t event);
};

extern MenuPage_t MainPage;
#endif