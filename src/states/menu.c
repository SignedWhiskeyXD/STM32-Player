#include "menu.h"

#include "player.h"
#include "states.h"

MenuItem selectedMenuItem = MENU_PLAYER;

MenuItem getSelectedMenuItem()
{
    return selectedMenuItem;
}

void moveMenuPointer(int8_t delta)
{
    if (delta > 0 && selectedMenuItem != MENU_RECORDER)
        ++selectedMenuItem;
    else if (delta < 0 && selectedMenuItem != MENU_PLAYER)
        --selectedMenuItem;
}

void setGlobalStateFromMenu()
{
    switch (selectedMenuItem) {
        case MENU_PLAYER:
            setGlobalState(BROWSING_DIR);
            break;
        case MENU_RECORDER:
            cancelPlayerTask();
            setGlobalState(RECORDING);
            break;
        default:
            break;
    }
}