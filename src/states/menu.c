#include "menu.h"

#include "states.h"
#include "player.h"

MenuItem selectedMenuItem = MENU_PLAYER;

MenuItem getSelectedMenuItem()
{
    return selectedMenuItem;
}

void moveMenuPointer(int8_t delta)
{    
    if(delta > 0 && selectedMenuItem != MENU_DRIVE)
        ++selectedMenuItem;
    else if(delta < 0 && selectedMenuItem != MENU_PLAYER)
        --selectedMenuItem;
}

void setGlobalStateFromMenu()
{
    switch (selectedMenuItem)
    {
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