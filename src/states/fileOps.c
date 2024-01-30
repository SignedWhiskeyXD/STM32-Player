#include "fileOps.h"

#include <string.h>

#include "FATFS/ff.h"

File_State ctx;

TCHAR lfnBuffer[256];

void toLower(char* filename)
{
    for(uint8_t i = 0; i < 13 && filename[i] != '\0'; ++i){
        if(filename[i] < 0)
            filename[i] = '?';
        else if(filename[i] >= 65 && filename[i] <= 90)
            filename[i] += 32;
    }
}

File_State* useFileState()
{
    return &ctx;
}

MYERROR initSD()
{
    FATFS fs;
    FRESULT res = f_mount(&fs, "0:", 1);
    if(res != FR_OK)
        return SD_FATFS_MOUNT_ERROR;
    return OPERATION_SUCCESS;
}

void loadFiles()
{
    DIR dir;
    FRESULT res = f_opendir(&dir, "0:/");

    FILINFO fileInfo;
    uint8_t idx = 0;
    fileInfo.lfname = lfnBuffer;
    fileInfo.lfsize = 256;
    while (1) {
        res = f_readdir(&dir, &fileInfo);
        if(res != FR_OK || fileInfo.fname[0] == '\0' || idx >= MAX_FILE_LIST_LENGTH) 
            break;

        if(!strstr(fileInfo.fname, ".MP3")) continue;
        
        strcpy(ctx.filenames[idx], fileInfo.fname);
        toLower(ctx.filenames[idx]);
        ++idx;
    }

    ctx.totalFiles = idx;
}

void moveFilePointer(int8_t delta)
{
    if (delta == 0) return;

    const int8_t newPos = ctx.filenameBase + ctx.offset + delta;
    
    if(newPos >= ctx.totalFiles || newPos < 0) return;

    if (delta > 0) {
        if (ctx.offset < 3) {
            ctx.offset++;
        } else if (ctx.filenameBase + 4 < MAX_FILE_LIST_LENGTH &&
                   ctx.filenames[ctx.filenameBase + 4][0] != '\0') {
            ctx.filenameBase++;
        }
    } else {
        if(ctx.offset > 0) ctx.offset--;
        else if(ctx.filenameBase > 0){
            ctx.filenameBase--;
        }
    }
}