#ifndef MENU_H
#define MENU_H

#include <stdint.h>

typedef enum {
    MENU_PLAYER,
    MENU_RECORDER
} MenuItem;

MenuItem getSelectedMenuItem();

void moveMenuPointer(int8_t delta);

void setGlobalStateFromMenu();

const char* getMenuName(MenuItem menuItem);

const uint8_t* getMenuFonts(MenuItem menuItem);

#endif