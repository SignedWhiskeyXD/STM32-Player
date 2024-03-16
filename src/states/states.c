#include "states.h"

static GlobalState currentState = PLAYER_START_UP;
static MyError     lastError    = OPERATION_SUCCESS;

GlobalState getGlobalState()
{
    return currentState;
}

void setGlobalState(GlobalState newState)
{
    currentState = newState;
}

MyError getLastError()
{
    return lastError;
}

void setLastError(MyError error)
{
    lastError = error;
}