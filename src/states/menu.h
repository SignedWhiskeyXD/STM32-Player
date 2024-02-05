#ifndef MENU_H
#define MENU_H

#include <stdint.h>

typedef enum {
    MENU_PLAYER,
    MENU_RECORDER,
    MENU_DRIVE
} MenuItem;

MenuItem getSelectedMenuItem();

void moveMenuPointer(int8_t delta);

void setGlobalStateFromMenu();

#endif