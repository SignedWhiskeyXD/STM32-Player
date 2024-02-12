#include <stdint.h>
#include <string.h>
#include "vs1053/VS1053.h"
#include "FATFS/ff.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"
#include "recorder.h"

#define NAMESIZE 30
static char last_record_pathname[NAMESIZE] = "0:Test_0001.wav";

#define TEMPDATA_SIZE  4096
#define RECORD_BUFSIZE 512

static FRESULT result;
static FIL file; /* file objects */

static __WaveHeader rechead;
static _recorder_obj recset = {
    0,  // MIC
    1,  // 8K
    0,  // ������
    6   // ����
};

uint32_t sectorsize            = 0;
int8_t tempdata[TEMPDATA_SIZE] = {0};
uint8_t recording = 1;

void stopRecording()
{
    UINT bw = 0;

    rechead.riff.ChunkSize = sectorsize * RECORD_BUFSIZE + 36;    // �����ļ��Ĵ�С-8;
    rechead.data.ChunkSize = sectorsize * RECORD_BUFSIZE;         // ���ݴ�С
    f_lseek(&file, 0);                                            // ƫ�Ƶ��ļ�ͷ.
    result = f_write(&file, &rechead, sizeof(__WaveHeader), &bw); // ��д��ͷ����
    f_close(&file);
    VS_HD_Reset();   // Ӳ��λ
    VS_Soft_Reset(); // ��λ

    sectorsize = 0;
}

void prepareRecorder()
{
    memset(tempdata, 0, TEMPDATA_SIZE);
    VS_Set_All();
    recoder_enter_rec_mode(&recset);

    while (VS_RD_Reg(SPI_HDAT1) >> 8); // �ȵ�buf ��Ϊ�����ٿ�ʼ

    rechead.riff.ChunkID      = 0X46464952;                 //"RIFF"
    rechead.riff.ChunkSize    = 0;                          // ��δȷ��,�����Ҫ����
    rechead.riff.Format       = 0X45564157;                 //"WAVE"
    rechead.fmt.ChunkID       = 0X20746D66;                 //"fmt "
    rechead.fmt.ChunkSize     = 16;                         // ��СΪ16���ֽ�
    rechead.fmt.AudioFormat   = 0X01;                       // 0X01,��ʾPCM;0X01,��ʾIMA ADPCM
    rechead.fmt.NumOfChannels = 1;                          // ������
    rechead.fmt.SampleRate    = recset.samplerate * 8000;   // ��������
    rechead.fmt.ByteRate      = rechead.fmt.SampleRate * 2; // 16λ,��2���ֽ�
    rechead.fmt.BlockAlign    = 2;                          // ���С,2���ֽ�Ϊһ����
    rechead.fmt.BitsPerSample = 16;                         // 16λPCM
    rechead.data.ChunkID      = 0X61746164;                 //"data"
    rechead.data.ChunkSize    = 0;                          // ���ݴ�С,����Ҫ����

    f_unlink(last_record_pathname);
    result = f_open(&file, last_record_pathname, FA_CREATE_ALWAYS | FA_WRITE);
    if (result != 0) {
        return;
    }
    UINT bw = 0;
    result = f_write(&file, &rechead, sizeof(__WaveHeader), &bw); // Ԥд��ͷ����
}

void onStateRecord()
{
    prepareRecorder();

    while (1) {
        uint16_t regval = VS_RD_Reg(SPI_HDAT1);
        uint16_t idx = 0;
        UINT bw = 0;

        if (regval < 256 || regval >= 896) continue;

        taskENTER_CRITICAL();
        while (idx < RECORD_BUFSIZE) // һ�ζ�ȡBUFSIZE�ֽ�
        {
            regval            = VS_RD_Reg(SPI_HDAT0);
            tempdata[idx]     = regval & 0XFF;
            tempdata[idx + 1] = regval >> 8;
            idx += 2;
        }
        f_lseek(&file, 44 + sectorsize * RECORD_BUFSIZE);
        
        result = f_write(&file, tempdata, RECORD_BUFSIZE, &bw); // д���ļ�
        taskEXIT_CRITICAL();

        if (result) {
            f_close(&file);
            return; // д�����.
        }
        sectorsize++; // ����������1

        vTaskDelay(10);

        if (!recording) {
            stopRecording();
            return;
        }
    }
}

TaskHandle_t recordTaskHandler;

void taskRecord()
{   
    onStateRecord();
    vTaskDelete(recordTaskHandler);
}

void toggleRecord()
{
    if(recording == 0){
        recording = 1;
        xTaskCreate(taskRecord, "taskRecord", 512, NULL, 3, &recordTaskHandler);
    }else{
        recording = 0;
    }
}

void stopRecorder()
{
    recording = 0;
}
