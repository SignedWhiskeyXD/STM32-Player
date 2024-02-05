#ifndef STATES_H
#define STATES_H

#include "fileOps.h"
#include "music.h"
#include "menu.h"

typedef enum {
    PLAYER_START_UP,
    BROWSING_MENU,
    BROWSING_DIR,
    
    PLAYER_ERROR
} GlobalState;

GlobalState getGlobalState();

void setGlobalState(GlobalState newState);

#endif