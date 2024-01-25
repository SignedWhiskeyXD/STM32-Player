#ifndef STATES_H
#define STATES_H

#include "fileOps.h"

typedef enum {
    PLAYER_START_UP,
    BORWSING_DIR,
    
    PLAYER_ERROR
} GlobalState;

GlobalState getGlobalState();

void setGlobalState(GlobalState newState);

#endif