#include "states.h"
#include "display.h"

GlobalState currentState = PLAYER_START_UP;

GlobalState getGlobalState()
{
    return currentState;
}

void setGlobalState(GlobalState newState)
{
    currentState = newState;
}