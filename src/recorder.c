#include <stdint.h>
#include <string.h>
#include "VS1053.h"
#include "recorder.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "ff.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"

#define NAMESIZE 30
static char last_record_pathname[NAMESIZE] = "0:Test_0001.wav";

#define TEMPDATA_SIZE  4096
#define RECORD_BUFSIZE 512
#define PLAY_BUFSIZE   1024 * 4

static FRESULT result;
static FIL file; /* file objects */

static __WaveHeader rechead;
static _recorder_obj recset;
static enumRecorder recorder_statue = STATE_WAIT;

uint32_t sectorsize            = 0;
uint16_t recsec                = 0; // ¼��ʱ��
uint8_t thisBuffer[PLAY_BUFSIZE]   = {0};
int8_t tempdata[TEMPDATA_SIZE] = {0};

void Rest_Statue(void)
{
    /* Ӳ��λMP3 */
    VS_HD_Reset();
    /* ��λVS10XX */
    VS_Soft_Reset();
    recorder_statue = STATE_WAIT;
}

void onStateWait()
{
    LED1_ON;

    if (Key_Scan(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == KEY_ON) {
        recorder_statue = STATE_RECORDING;
    }

    if (Key_Scan(KEY2_GPIO_PORT, KEY2_GPIO_PIN) == KEY_ON) {
        recorder_statue = STATE_PLAYING;
    }
    vTaskDelay(20);
}

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

    recsec = sectorsize * 4 / 125 / recset.samplerate;
    sectorsize = 0;

    recorder_statue = STATE_WAIT;
}

void Recorder_Run()
{
    LED1_OFF;
    LED2_OFF;
    LED3_OFF;

    while (1) {
        switch (recorder_statue) {
            case STATE_WAIT:
                onStateWait();
                break;

            case STATE_RECORDING:

                LED1_OFF;

                memset(tempdata, 0, TEMPDATA_SIZE);

                vsset.mvol    = 240; // ������С
                vsset.bflimit = 6;   // ��������
                vsset.bass    = 15;  // ����Ч��
                vsset.tflimit = 10;  // ��������
                vsset.treble  = 15;  // ����Ч��
                vsset.effect  = 0;   // �޶���Ч��
                VS_Set_All();

                recset.input      = 0; // MIC
                recset.samplerate = 1; // 8K
                recset.channel    = 0; // ������
                recset.agc        = 6; // ����
                recoder_enter_rec_mode(&recset);

                while (VS_RD_Reg(SPI_HDAT1) >> 8)
                    ; // �ȵ�buf ��Ϊ�����ٿ�ʼ

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
                    Rest_Statue();
                    return;
                }
                UINT bw = 0;
                result = f_write(&file, &rechead, sizeof(__WaveHeader), &bw); // Ԥд��ͷ����

                while (1) {
                    LED2_TOGGLE;
                    uint16_t regval = VS_RD_Reg(SPI_HDAT1);
                    if ((regval >= 256) && (regval < 896)) {
                        uint16_t idx = 0;
                        while (idx < RECORD_BUFSIZE) // һ�ζ�ȡBUFSIZE�ֽ�
                        {
                            regval            = VS_RD_Reg(SPI_HDAT0);
                            tempdata[idx]     = regval & 0XFF;
                            tempdata[idx + 1] = regval >> 8;
                            idx += 2;
                        }
                        f_lseek(&file, 44 + sectorsize * RECORD_BUFSIZE);
                        result = f_write(&file, tempdata, RECORD_BUFSIZE, &bw); // д���ļ�
                        if (result) {
                            f_close(&file);
                            Rest_Statue();
                            return; // д�����.
                        }
                        sectorsize++; // ����������1
                    }

                    if (Key_Scan(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == KEY_ON) {
                        stopRecording();
                        return;
                    }
                }

                break;

            case STATE_PLAYING:

                LED2_OFF;

                vsset.mvol    = 240; // ������С
                vsset.bflimit = 6;   // ��������
                vsset.bass    = 15;  // ����Ч��
                vsset.tflimit = 10;  // ��������
                vsset.treble  = 15;  // ����Ч��
                vsset.effect  = 0;   // �޶���Ч��

                VS_Restart_Play();
                VS_Set_All();
                VS_Reset_DecodeTime();
                VS_SPI_SpeedHigh();

                result = f_open(&file, last_record_pathname, FA_READ);
                if (result != 0) {
                    Rest_Statue();
                    return;
                }

                while (1) {
                    LED2_TOGGLE;
                    result = f_read(&file, thisBuffer, PLAY_BUFSIZE, (UINT *)&bw);
                    if (result != 0) {
                        f_close(&file);

                        Rest_Statue();
                        return;
                    }

                    uint16_t i = 0;
                    do {
                        if (VS_Send_MusicData(thisBuffer + i) == 0) {
                            i += 32;
                        }

                        if (Key_Scan(KEY2_GPIO_PORT, KEY2_GPIO_PIN) == KEY_ON) {
                            f_close(&file);
                            // printf("ֹͣ����\r\n");
                            recorder_statue = STATE_WAIT;
                            return;
                        }

                    } while (i < bw);

                    if (bw != PLAY_BUFSIZE || result != 0) {
                        f_close(&file);
                        // printf("���Ž���\r\n");
                        recorder_statue = STATE_WAIT;
                        return;
                    }
                }

                break;

            default:
                break;
        }
    }
}

void taskRecord()
{   
    while(1)
    {
        Recorder_Run();
    }
}

TaskHandle_t recordTaskHandler;

void startRecord()
{
    xTaskCreate(taskRecord, "taskRecord", 512, NULL, 3, &recordTaskHandler);
}