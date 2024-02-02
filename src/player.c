#include "player.h"

#include <string.h>
#include "states/fileOps.h"
#include "vs1053/VS1053.h"
#include "ff.h"

FRESULT result;
FIL file;
UINT bw;

static uint8_t buffer[BUFSIZE];
char Restart_Play_flag = 0;
// 播放歌曲
void vs1053_player_song(char* filepath)
{
    uint16_t i = 0;

    VS_Restart_Play();
    VS_Set_All();
    VS_Reset_DecodeTime();

    result = f_open(&file, (const TCHAR *)filepath, FA_READ);

    if (result == 0) {
        VS_SPI_SpeedHigh();
        while (1) {
            i      = 0;
            result = f_read(&file, buffer, BUFSIZE, (UINT *)&bw);
            do {
                if (VS_Send_MusicData(buffer + i) == 0) {
                    i += 32;
                    if (Restart_Play_flag) {
                        VS_Restart_Play();
                        Restart_Play_flag = 0;
                        goto exit;
                    }
                }
            } while (i < bw);

            if (bw != BUFSIZE || result != 0) {
                break;
            }
        }
    exit:
        f_close(&file);
    }
}

void playSelectedSong()
{
    File_State* fileState = useFileState();
    char musicFile[16] = "0:/";

    const uint8_t selectedIndex = fileState->filenameBase + fileState->offset;
    strcpy(musicFile + 3, fileState->filenames[selectedIndex]);

    vs1053_player_song(musicFile);
}