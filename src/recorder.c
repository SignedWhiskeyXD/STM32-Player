#include "recorder.h"
#include "FATFS/ff.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"
#include "vs1053/vs1053.h"
#include "vs1053/wav.h"
#include <string.h>

#define NAMESIZE       30
#define TEMPDATA_SIZE  4096
#define RECORD_BUFSIZE 512

static char last_record_pathname[NAMESIZE] = "0:/Test_0001.wav";

static FRESULT result;
static FIL     recordFile; /* file objects */

static WavHeader     rechead;
static RecordSetting recset = {
    0, // MIC
    1, // 8K
    0, // 左声道
    6  // 增益
};

uint32_t sectorSize = 0;
uint8_t  recording  = 0;
uint8_t  recordBuffer[TEMPDATA_SIZE];

TaskHandle_t recordTaskHandler;

void stopRecording()
{
    UINT bw = 0;

    rechead.riff.ChunkSize = sectorSize * RECORD_BUFSIZE + 36;       // 整个文件的大小-8;
    rechead.data.ChunkSize = sectorSize * RECORD_BUFSIZE;            // 数据大小
    f_lseek(&recordFile, 0);                                         // 偏移到文件头.
    result = f_write(&recordFile, &rechead, sizeof(WavHeader), &bw); // 覆写入头数据
    f_close(&recordFile);
    VS_HD_Reset();                                                   // 硬复位
    VS_Soft_Reset();                                                 // 软复位

    sectorSize = 0;
}

void prepareRecorder()
{
    f_unlink(last_record_pathname);
    result = f_open(&recordFile, last_record_pathname, FA_CREATE_ALWAYS | FA_WRITE);
    if (result != FR_OK) {
        return;
    }

    memset(recordBuffer, 0, TEMPDATA_SIZE);
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
    result  = f_write(&recordFile, &rechead, sizeof(WavHeader), &bw); // 预写入头数据
}

void onStateRecord()
{
    prepareRecorder();

    while (1) {
        uint16_t regVal = VS_RD_Reg(SPI_HDAT1);
        uint16_t idx    = 0;
        UINT     bw     = 0;

        if (regVal < 256 || regVal >= 896) continue;

        while (idx < RECORD_BUFSIZE) {
            regVal                = VS_RD_Reg(SPI_HDAT0);
            recordBuffer[idx]     = regVal & 0XFF;
            recordBuffer[idx + 1] = regVal >> 8;
            idx += 2;
        }
        f_lseek(&recordFile, 44 + sectorSize * RECORD_BUFSIZE);

        result = f_write(&recordFile, recordBuffer, RECORD_BUFSIZE, &bw); // 写入文件

        if (result) {
            f_close(&recordFile);
            return;   // 写入出错.
        }
        sectorSize++; // 扇区数增加1

        vTaskDelay(10);

        if (!recording) {
            stopRecording();
            return;
        }
    }
}

void taskRecord()
{
    onStateRecord();
    vTaskDelete(recordTaskHandler);
}

void toggleRecord()
{
    if (recording == 0) {
        recording = 1;
        xTaskCreate(taskRecord, "taskRecord", 512, NULL, 3, &recordTaskHandler);
    } else {
        recording = 0;
    }
}

void stopRecorder()
{
    recording = 0;
}
