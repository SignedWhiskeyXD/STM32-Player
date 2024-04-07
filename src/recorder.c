#include "recorder.h"
#include "FATFS/ff.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"
#include "vs1053/vs1053.h"
#include "vs1053/wav.h"
#include <string.h>

#define NAMESIZE       30
#define RECORD_BUFSIZE 512

static char last_record_pathname[NAMESIZE] = "0:/Test_0001.wav";

static FRESULT result     = FR_OK;
static FIL*    recordFile = NULL;

static WavHeader     rechead;
static RecordSetting recset = {
    0, // MIC
    1, // 8K
    0, // 左声道
    6  // 增益
};

uint32_t sectorSize   = 0;
uint8_t  recording    = 0;
uint8_t* recordBuffer = NULL;

TaskHandle_t recordTaskHandler;

void stopRecording()
{
    UINT bw = 0;

    rechead.riff.ChunkSize = sectorSize * RECORD_BUFSIZE + 36;      // 整个文件的大小-8;
    rechead.data.ChunkSize = sectorSize * RECORD_BUFSIZE;           // 数据大小

    taskENTER_CRITICAL();
    f_lseek(recordFile, 0);                                         // 偏移到文件头.
    f_write(recordFile, &rechead, sizeof(WavHeader), &bw); // 覆写入头数据
    f_close(recordFile);
    taskEXIT_CRITICAL();

    VS_HD_Reset();                                                  // 硬复位
    VS_Soft_Reset();                                                // 软复位

    sectorSize = 0;
}

void prepareRecorder()
{
    taskENTER_CRITICAL();
    f_unlink(last_record_pathname);
    result = f_open(recordFile, last_record_pathname, FA_CREATE_ALWAYS | FA_WRITE);
    taskEXIT_CRITICAL();

    if (result != FR_OK) {
        return;
    }

    VS_Set_All();
    VS_StartRecord(&recset);

    while (VS_RD_Reg(SPI_HDAT1) >> 8)
        ;                                                   // 等到buf 较为空闲再开始

    rechead.riff.ChunkID      = 0X46464952;                 //"RIFF"
    rechead.riff.ChunkSize    = 0;                          // 还未确定,最后需要计算
    rechead.riff.Format       = 0X45564157;                 //"WAVE"
    rechead.fmt.ChunkID       = 0X20746D66;                 //"fmt "
    rechead.fmt.ChunkSize     = 16;                         // 大小为16个字节
    rechead.fmt.AudioFormat   = 0X01;                       // 0X01,表示PCM;0X01,表示IMA ADPCM
    rechead.fmt.NumOfChannels = 1;                          // 单声道
    rechead.fmt.SampleRate    = recset.sampleRate * 8000;   // 采样速率
    rechead.fmt.ByteRate      = rechead.fmt.SampleRate * 2; // 16位,即2个字节
    rechead.fmt.BlockAlign    = 2;                          // 块大小,2个字节为一个块
    rechead.fmt.BitsPerSample = 16;                         // 16位PCM
    rechead.data.ChunkID      = 0X61746164;                 //"data"
    rechead.data.ChunkSize    = 0;                          // 数据大小,还需要计算

    UINT bw = 0;

    taskENTER_CRITICAL();
    result  = f_write(recordFile, &rechead, sizeof(WavHeader), &bw); // 预写入头数据
    taskEXIT_CRITICAL();
}

void onStateRecord()
{
    prepareRecorder();

    while (1) {
        uint16_t regVal = VS_RD_Reg(SPI_HDAT1);
        UINT     bw     = 0;

        if (regVal < 256 || regVal >= 896) continue;

        for(uint16_t idx = 0; idx + 1 < RECORD_BUFSIZE; idx += 2) {
            regVal                = VS_RD_Reg(SPI_HDAT0);
            recordBuffer[idx]     = regVal & 0XFF;
            recordBuffer[idx + 1] = regVal >> 8;
        }

        taskENTER_CRITICAL();
        f_lseek(recordFile, 44 + sectorSize * RECORD_BUFSIZE);
        result = f_write(recordFile, recordBuffer, RECORD_BUFSIZE, &bw); // 写入文件

        if (result != FR_OK) {
            f_close(recordFile);
            taskEXIT_CRITICAL();
            return;
        }
        taskEXIT_CRITICAL();

        sectorSize++; // 扇区数增加1
        vTaskDelay(10);

        if (!recording) {
            stopRecording();
            return;
        }
    }
}

static uint8_t allocateRecorderBuffer()
{
    /* 已分配内存则无需分配 */
    if (recordBuffer != NULL && recordFile != NULL) return 0;

    if (recordFile == NULL) {
        if ((recordFile = pvPortMalloc(sizeof(FIL))) == NULL) return 1;
        memset(recordFile, 0, sizeof(FIL));
    }

    if (recordBuffer == NULL) {
        if ((recordBuffer = pvPortMalloc(RECORD_BUFSIZE)) == NULL) return 1;
        memset(recordBuffer, 0, RECORD_BUFSIZE);
    }

    return 0;
}

static void freeRecorderBuffer()
{
    if (recordFile != NULL) {
        vPortFree(recordFile);
        recordFile = NULL;
    }

    if (recordBuffer != NULL) {
        vPortFree(recordBuffer);
        recordBuffer = NULL;
    }
}

void taskRecord()
{
    onStateRecord();

    freeRecorderBuffer();
    vTaskDelete(recordTaskHandler);
}

void toggleRecord()
{
    if (recording == 0) {
        if (allocateRecorderBuffer() != 0) return;

        const BaseType_t taskStatus = xTaskCreate(taskRecord, "taskRecord", 128, NULL, 3, &recordTaskHandler);
        if(taskStatus != pdPASS) return;

        recording = 1;
    } else {
        recording = 0;
    }
}

void stopRecorder()
{
    recording = 0;
}
