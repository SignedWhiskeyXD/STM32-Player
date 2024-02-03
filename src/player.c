#include "player.h"

#include <string.h>
#include "states/fileOps.h"
#include "display.h"
#include "vs1053/VS1053.h"
#include "ff.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"

uint8_t buffer[BUFSIZE];
FIL musicFile;

TaskHandle_t taskMusicHandler = NULL;

void doBufferTransfer(UINT bufferLength)
{
    uint16_t offset = 0;
    while (offset < bufferLength) {
        if (VS_Send_MusicData(buffer + offset) != 0) continue;
        offset += 32;
    }
}

void taskPlayMusic(void* filepath)
{
    VS_Restart_Play();
    VS_Set_All();
    VS_Reset_DecodeTime();

    FRESULT result = f_open(&musicFile, (const TCHAR *)filepath, FA_READ);

    if (result != FR_OK) {
        vTaskDelete(taskMusicHandler);
        return;
    } 
    
    VS_SPI_SpeedHigh();
    while (1) {
        UINT bufferUsed;

        // 必须确保SDIO操作是原子的
        taskENTER_CRITICAL();
        result = f_read(&musicFile, buffer, BUFSIZE, &bufferUsed);
        taskEXIT_CRITICAL();

        doBufferTransfer(bufferUsed);

        if (bufferUsed != BUFSIZE || result != FR_OK) {
            break;
        }

        vTaskDelay(30);
    }
    f_close(&musicFile);

    File_State* fileState = useFileState();
    fileState->nowPlaying = fileState->totalFiles;
    refreshScreen();

    vTaskDelete(taskMusicHandler);
}

void playSelectedSong()
{
    File_State* fileState = useFileState();
    char musicFile[16] = "0:/";

    const uint8_t selectedIndex = fileState->filenameBase + fileState->offset;
    strcpy(musicFile + 3, fileState->filenames[selectedIndex]);

    // 如果播放任务还没完成，只是在等待期间阻塞了，那么需要先行删除
    const eTaskState taskState = eTaskGetState(taskMusicHandler);
    if(taskState == eBlocked)
        vTaskDelete(taskMusicHandler);
    
    fileState->nowPlaying = selectedIndex;
    fileState->paused = 0;
    refreshScreen();
    xTaskCreate(taskPlayMusic, "MusicPlay", 512, musicFile, 3, &taskMusicHandler);
}

uint8_t pauseOrResumeSelectedSong()
{
    File_State* fileState = useFileState();
    const uint8_t selectedIndex = fileState->filenameBase + fileState->offset;

    if(selectedIndex != fileState->nowPlaying) return 1;

    const eTaskState taskState = eTaskGetState(taskMusicHandler);
    if(taskState == eBlocked) {
        fileState->paused = 1;
        vTaskSuspend(taskMusicHandler);
    }
    else if(taskState == eSuspended) {
        fileState->paused = 0;
        vTaskResume(taskMusicHandler);
    }
    refreshScreen();
    return 0;
}