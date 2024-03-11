#include "display.h"

#include <stdio.h>
#include "myError.h"
#include "states/states.h"
#include "oled/OLED.h"

void initScreen()
{
    OLED_Init();
}

void showStartUp()
{
    OLED_ShowPaddingString(0, 0, "Starting", 16);
}

void showMenuBrowsing()
{
    const MenuItem selectedMenuItem = getSelectedMenuItem();
    for(uint8_t i = 0; i < 3; ++i) {
        OLED_ShowChar(i, 0, i == selectedMenuItem ? '>' : ' ');
        OLED_ShowGBKString(i, 1, 15, (char*)getMenuName(i), (uint8_t*)getMenuFonts(i));
    }
}

void showDirectoryBrowsing()
{
    File_State* fileState = useFileState();

    uint8_t lines = fileState->totalFiles - fileState->filenameBase;
    if(lines > DIR_MAX_LINES)
        lines = DIR_MAX_LINES;

    for(uint8_t i = 0; i < lines; ++i){
        if(fileState->filenameBase + i == fileState->nowPlaying)
            OLED_ShowChar(i, 0, fileState->paused ? '-' : '+');
        else if(i == fileState->offset)
            OLED_ShowChar(i, 0, '>');
        else
            OLED_ShowChar(i, 0, ' ');

        OLED_ShowPaddingString(i, 1, fileState->filenames[fileState->filenameBase + i], 15);
    }
}

void showProgress()
{
    static char progressBuffer[17];
    const MusicState* musicState = useMusicState();

    if(musicState->musicSize == 0){
        OLED_ShowPaddingString(3, 0, "", 16);
        return;
    }

    if(musicState->avgByteRate == 0){
        OLED_ShowPaddingString(3, 2, "Loading...", 14);
        return;
    }

    const uint16_t musicLength = musicState->musicSize / musicState->avgByteRate;
    const uint16_t currentProgress = musicState->decodeTime + musicState->offsetTime;
    sprintf(progressBuffer, "%02d:%02d/%02d:%02d",
            currentProgress / 60,
            currentProgress % 60,
            musicLength / 60,
            musicLength % 60);

    OLED_ShowString(3, 2, progressBuffer);
}

void showRecorder()
{
    OLED_ShowPaddingString(0, 0, "Recorder", 16);
    for(uint8_t i = 1; i < 4; ++i)
        OLED_ShowPaddingString(i, 0, "", 16);
}

void showError()
{
    switch (getLastError()) {
        case SD_FATFS_MOUNT_ERROR:
            OLED_ShowPaddingString(0, 0, "SD Mount Failed", 16);
            break;

        default:
            break;
    }
}

void onScreenRefresh()
{
    switch (getGlobalState()) {
        case PLAYER_START_UP:
            showStartUp();
            break;

        case BROWSING_MENU:
            showMenuBrowsing();
            showProgress();
            break;

        case BROWSING_DIR:
            showDirectoryBrowsing();
            showProgress();
            break;
        
        case RECORDING:
            showRecorder();
            break;

        case PLAYER_ERROR:
            showError();
            break;
    
        default:
            return;
    }
    OLED_flushScreen();
}