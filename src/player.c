#include "player.h"

#include <string.h>
#include "states/fileOps.h"
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

        result = f_read(&musicFile, buffer, BUFSIZE, &bufferUsed);
        doBufferTransfer(bufferUsed);

        if (bufferUsed != BUFSIZE || result != FR_OK) {
            break;
        }

        vTaskDelay(30);
    }
    f_close(&musicFile);
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
    
    xTaskCreate(taskPlayMusic, "MusicPlay", 512, musicFile, 3, &taskMusicHandler);
}