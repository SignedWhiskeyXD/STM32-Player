#ifndef STATES_H
#define STATES_H

typedef enum {
    PLAYER_START_UP,
    BROWSING_MENU,
    BROWSING_DIR,
    RECORDING,

    PLAYER_ERROR
} GlobalState;

typedef enum {
    OPERATION_SUCCESS,
    SD_FATFS_MOUNT_ERROR,
    VS_RAM_TEST_FAILED
} MyError;

GlobalState getGlobalState();

void setGlobalState(GlobalState newState);

MyError getLastError();

void setLastError(MyError error);

#include "fileOps.h"
#include "menu.h"
#include "music.h"

#endif