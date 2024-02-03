#include "display.h"

#include <stdio.h>
#include "myError.h"
#include "stm32f10x.h"
#include "states/states.h"
#include "oled/OLED.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"

void initScreen()
{
    OLED_Init();
}

void showStartUp()
{
    OLED_ShowString(1, 1, "Starting");
}

void showDirectoryBrowsing()
{
    File_State* fileState = useFileState();

    uint8_t lines = fileState->totalFiles - fileState->filenameBase;
    if(lines > DIR_MAX_LINES)
        lines = DIR_MAX_LINES;

    for(uint8_t i = 0; i < lines; ++i){
        if(fileState->filenameBase + i == fileState->nowPlaying)
            OLED_ShowChar(i + 1, 1, fileState->paused ? '-' : '+');
        else if(i == fileState->offset)
            OLED_ShowChar(i + 1, 1, '>');
        else
            OLED_ShowChar(i + 1, 1, ' ');

        OLED_ShowPaddingString(i, 1, fileState->filenames[fileState->filenameBase + i], 15);
    }
}

void showProgress()
{
    static char progressBuffer[17];
    MusicState* musicState = useMusicState();

    if(musicState->musicSize == 0){
        OLED_ShowPaddingString(3, 0, "", 16);
        return;
    }

    if(musicState->avgBitrate == 0){
        OLED_ShowPaddingString(3, 2, "Loading...", 13);
        return;
    }

    const uint16_t leftLength = musicState->musicSize / musicState->avgBitrate - musicState->decodeTime;
    sprintf(progressBuffer, "%02d:%02d/%02d:%02d",
            musicState->decodeTime / 60,
            musicState->decodeTime % 60,
            leftLength / 60,
            leftLength % 60);

    OLED_ShowString(4, 3, progressBuffer);
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
    GlobalState currentState = getGlobalState();
    if (currentState != BROWSING_DIR)
        OLED_Clear();

    switch (currentState) {
        case PLAYER_START_UP:
            showStartUp();
            break;

        case BROWSING_DIR:
            showDirectoryBrowsing();
            showProgress();
            break;

        case PLAYER_ERROR:
            showError();
            break;
    
        default:
            break;
    }
}