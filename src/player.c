#include "player.h"

#include <string.h>
#include "states/states.h"
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
        // 如果DREQ没有就绪，不要进行忙等待，应让出CPU
        if (VS_Send_MusicData(buffer + offset) != 0) {
            vTaskDelay(5);
            continue;
        }
        offset += 32;
    }
}

uint32_t getMusicHeaderSize()
{
    /*
        对于MP3音频，除了文件尾部的ID2标签外，也有可能会在文件首部包含ID3标签
        ID3标签包含了很多元信息，包括曲目的封面图片，这些数据对硬件解码而言是无用的
        应当从ID3首部中计算出首部长度，得出文件指针偏移量，免得浪费时间
    */
    UINT bufferUsed;
    FRESULT res = f_read(&musicFile, buffer, 10, &bufferUsed);
    
    if(res != FR_OK || bufferUsed != 10) return 0;

    if(buffer[0] != 'I' || buffer[1] != 'D' || buffer[2] != '3') return 0;

    uint32_t headerLength = 0;
    for(uint8_t i = 0; i < 4; ++i)
        headerLength |= (buffer[i + 6] << (21 - i * 7));
    
    return headerLength > musicFile.fsize ? 0 : headerLength;
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
    
    MusicState* musicState = useMusicState();
    resetMusicState();

    uint32_t headerLength = getMusicHeaderSize();
    f_lseek(&musicFile, headerLength);
    musicState->musicSize = musicFile.fsize - headerLength;

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

        musicState->avgBitrate = VS_Get_ByteRate();
        musicState->decodeTime = VS_Get_DecodeTime();

        vTaskDelay(20);
    }
    f_close(&musicFile);

    File_State* fileState = useFileState();
    fileState->nowPlaying = fileState->totalFiles;
    resetMusicState();

    vTaskDelete(taskMusicHandler);
}

void playSelectedSong()
{
    File_State* fileState = useFileState();
    char musicFile[16] = "0:/";

    const uint8_t selectedIndex = fileState->filenameBase + fileState->offset;
    strcpy(musicFile + 3, fileState->filenames[selectedIndex]);

    // 如果播放任务还没完成，只是在等待期间阻塞或挂起了，那么需要先行删除
    const eTaskState taskState = eTaskGetState(taskMusicHandler);
    if(taskState == eBlocked || taskState == eSuspended)
        vTaskDelete(taskMusicHandler);
    
    fileState->nowPlaying = selectedIndex;
    fileState->paused = 0;
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
    return 0;
}