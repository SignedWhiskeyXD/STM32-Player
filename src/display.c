#include "display.h"

#include "myError.h"
#include "stm32f10x.h"
#include "states/states.h"
#include "oled/OLED.h"

uint8_t shouldRefresh = 1;

void initScreen()
{
    OLED_Init();
}

void refreshScreen()
{
    shouldRefresh = 1;
}

void showStartUp()
{
    OLED_ShowString(1, 1, "Starting");
}

void showDirectoryBrowsing()
{
    File_State* fileState = useFileState();

    const uint8_t lines = fileState->totalFiles - fileState->filenameBase;

    for(uint8_t i = 0; i < lines; ++i){
        if(fileState->filenameBase + i == fileState->nowPlaying)
            OLED_ShowChar(i + 1, 1, fileState->paused ? '-' : '+');
        else if(i == fileState->offset)
            OLED_ShowChar(i + 1, 1, '>');
        else
            OLED_ShowChar(i + 1, 1, ' ');

        OLED_ShowString(i + 1, 2, fileState->filenames[fileState->filenameBase + i]);
    }
}

void showError()
{
    switch (getLastError()) {
        case SD_FATFS_MOUNT_ERROR:
            OLED_ShowString(1, 1, "SD Mount Failed");
            break;

        default:
            break;
    }
}

void onScreenRefresh()
{
    if(shouldRefresh == 0) {
        return;
    }

    OLED_Clear();
    switch (getGlobalState()) {
        case PLAYER_START_UP:
            showStartUp();
            break;

        case BORWSING_DIR:
            showDirectoryBrowsing();
            break;

        case PLAYER_ERROR:
            showError();
            break;
    
        default:
            break;
    }

    shouldRefresh = 0;
}