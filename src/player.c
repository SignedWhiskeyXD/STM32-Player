#include "player.h"

#include <string.h>
#include "states/fileOps.h"
#include "vs1053/VS1053.h"
#include "ff.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"

uint8_t buffer[BUFSIZE];
FIL musicFile;

TaskHandle_t taskMusicHandler;

void doBufferTransfer(UINT bufferLength)
{
    uint16_t offset = 0;
    while (offset < bufferLength) {
        if (VS_Send_MusicData(buffer + offset) != 0) continue;
        offset += 32;
    }
}

void vs1053_player_song(void* filepath)
{
    VS_Restart_Play();
    VS_Set_All();
    VS_Reset_DecodeTime();

    FRESULT result = f_open(&musicFile, (const TCHAR *)filepath, FA_READ);

    if (result != FR_OK) return; 
    
    VS_SPI_SpeedHigh();
    while (1) {
        UINT bw;

        result = f_read(&musicFile, buffer, BUFSIZE, &bw);
        doBufferTransfer(bw);

        if (bw != BUFSIZE || result != FR_OK) {
            break;
        }

        vTaskDelay(30);
    }
    f_close(&musicFile);
}

void playSelectedSong()
{
    File_State* fileState = useFileState();
    char musicFile[16] = "0:/";

    const uint8_t selectedIndex = fileState->filenameBase + fileState->offset;
    strcpy(musicFile + 3, fileState->filenames[selectedIndex]);

    xTaskCreate(vs1053_player_song, "MusicPlay", 512, musicFile, 3, &taskMusicHandler);
}