#include "player.h"

#include "FatFs/ff.h"
#include "daemon_tasks.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"
#include "states/states.h"
#include "vs1053/vs1053.h"
#include <string.h>

static uint8_t* playerBuffer = NULL;
static FIL*     musicFile = NULL;
static int8_t   jumpFlag  = 0;

static TaskHandle_t taskMusicHandler = NULL;

static uint8_t allocateMusicBuffer()
{
    /* 已分配内存则无需分配，发生于切歌时 */
    if (playerBuffer != NULL && musicFile != NULL) return 0;

    if (musicFile == NULL) {
        if ((musicFile = pvPortMalloc(sizeof(FIL))) == NULL) return 1;
        memset(musicFile, 0, sizeof(FIL));
    }

    if (playerBuffer == NULL) {
        if ((playerBuffer = pvPortMalloc(BUFSIZE)) == NULL) return 1;
        memset(playerBuffer, 0, BUFSIZE);
    }

    return 0;
}

static void freePlayerBuffer()
{
    if(musicFile != NULL){
        vPortFree(musicFile);
        musicFile = NULL;
    }

    if(playerBuffer != NULL){
        vPortFree(playerBuffer);
        playerBuffer = NULL;
    }
}

static void doBufferTransfer(UINT bufferLength)
{
    uint16_t offset = 0;
    while (offset < bufferLength) {
        // 如果DREQ没有就绪，不要进行忙等待，应让出CPU
        if (VS_Send_MusicData(playerBuffer + offset) != 0) {
            vTaskDelay(5);
            continue;
        }
        offset += 32;
    }
}

static uint32_t getMusicHeaderSize()
{
    /*
        对于MP3音频，除了文件尾部的ID2标签外，也有可能会在文件首部包含ID3标签
        ID3标签包含了很多元信息，包括曲目的封面图片，这些数据对硬件解码而言是无用的
        应当从ID3首部中计算出首部长度，得出文件指针偏移量，免得浪费时间
    */
    UINT    bufferUsed;
    FRESULT res = f_read(musicFile, playerBuffer, 10, &bufferUsed);

    if (res != FR_OK || bufferUsed != 10 || strncmp((char*) playerBuffer, "ID3", 3) != 0) return 0;

    uint32_t headerLength = 0;
    for (uint8_t i = 0; i < 4; ++i) headerLength |= (playerBuffer[i + 6] << (21 - i * 7));

    return headerLength > musicFile->fsize ? 0 : headerLength;
}

static void doMusicJump()
{
    taskENTER_CRITICAL();
    if (VS_MusicJump() != 0) {
        taskEXIT_CRITICAL();
        return;
    }

    MusicState* musicState = useMusicState();
    uint32_t    absDelta   = musicState->avgByteRate * 5; // 进/退5秒钟

    if (jumpFlag > 0) {
        f_lseek(musicFile, musicFile->fptr + absDelta);
        musicState->offsetTime += 5;
    } else if (jumpFlag < 0 && absDelta <= musicFile->fptr) {
        f_lseek(musicFile, musicFile->fptr - absDelta);
        musicState->offsetTime -= 5;
    } else if (jumpFlag < 0) {
        // 倒回开始了，不如直接重设解码时间
        f_lseek(musicFile, musicFile->fsize - musicState->musicSize);
        musicState->offsetTime = 0;
        VS_Reset_DecodeTime();
    }

    jumpFlag = 0;
    taskEXIT_CRITICAL();
}

void taskPlayMusic(void* filepath)
{
    VS_Restart_Play();
    VS_Set_All();
    VS_Reset_DecodeTime();

    FRESULT result = f_open(musicFile, (const TCHAR*) filepath, FA_READ);

    if (result != FR_OK) {
        vTaskDelete(taskMusicHandler);
        return;
    }

    MusicState* musicState = useMusicState();
    resetMusicState();

    uint32_t headerLength = getMusicHeaderSize();
    f_lseek(musicFile, headerLength);
    musicState->musicSize = musicFile->fsize - headerLength;

    VS_SPI_SpeedHigh();
    while (1) {
        UINT bufferUsed;

        /* 必须屏蔽LCD的DMA传输完成回调中断，不可打断SDIO操作，否则读取会出错 */
        taskENTER_CRITICAL();
        result = f_read(musicFile, playerBuffer, BUFSIZE, &bufferUsed);
        taskEXIT_CRITICAL();

        doBufferTransfer(bufferUsed);

        if (bufferUsed != BUFSIZE || result != FR_OK) {
            break;
        }

        musicState->avgByteRate = VS_Get_ByteRate();

        const uint16_t currentDecodeTime = VS_Get_DecodeTime();
        if (currentDecodeTime != musicState->decodeTime) {
            musicState->decodeTime = currentDecodeTime;
            notifyScreenRefresh();
        }

        if (jumpFlag != 0) doMusicJump();

        vTaskDelay(20);
    }
    f_close(musicFile);

    File_State* fileState = useFileState();
    fileState->nowPlaying = fileState->totalFiles;

    resetMusicState();
    notifyScreenRefresh();

    freePlayerBuffer();
    vTaskDelete(taskMusicHandler);
}

void playSelectedSong()
{
    if (allocateMusicBuffer() != 0) return;

    File_State* fileState                     = useFileState();
    TCHAR       musicFilename[MAX_LFN_LENGTH] = "0:/";

    const uint8_t selectedIndex = fileState->filenameBase + fileState->offset;
    strcpy(musicFilename + 3, fileState->filenames[selectedIndex]);

    // 如果播放任务还没完成，只是在等待期间阻塞或挂起了，那么需要先行删除
    const eTaskState taskState = eTaskGetState(taskMusicHandler);
    if (taskState == eBlocked || taskState == eSuspended) vTaskDelete(taskMusicHandler);

    fileState->nowPlaying = selectedIndex;
    fileState->paused     = 0;

    xTaskCreate(taskPlayMusic, "MusicPlay", 512, musicFilename, 3, &taskMusicHandler);
}

uint8_t pauseOrResumeSelectedSong()
{
    File_State*   fileState     = useFileState();
    const uint8_t selectedIndex = fileState->filenameBase + fileState->offset;

    if (selectedIndex != fileState->nowPlaying) return 1;

    const eTaskState taskState = eTaskGetState(taskMusicHandler);
    if (taskState == eBlocked) {
        fileState->paused = 1;
        vTaskSuspend(taskMusicHandler);
    } else if (taskState == eSuspended) {
        fileState->paused = 0;
        vTaskResume(taskMusicHandler);
    }
    return 0;
}

void setJumpFlag(int8_t direction)
{
    const File_State* fileState = useFileState();
    // 未播放歌曲，或者暂停播放时，不进行进度调整
    if (fileState->nowPlaying > fileState->totalFiles || fileState->paused) return;

    jumpFlag = direction;
}

void cancelPlayerTask()
{
    const eTaskState taskState = eTaskGetState(taskMusicHandler);
    if (taskState == eBlocked || taskState == eSuspended) vTaskDelete(taskMusicHandler);

    freePlayerBuffer();

    File_State* fileState = useFileState();
    fileState->nowPlaying = fileState->totalFiles;
    fileState->paused     = 0;
    resetMusicState();
}