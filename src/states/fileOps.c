#include "fileOps.h"

#include <string.h>

#include "FATFS/ff.h"
#include "states/fileOps.h"

File_State ctx;

void toLower(char* filename)
{
    for(uint8_t i = 0; i < 12 && filename[i] != '\0'; ++i){
        if(filename[i] >= 65 && filename[i] <= 90)
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
    f_opendir(&dir, "0:/");

    FILINFO fileInfo;
    uint8_t index = 0;
    while (1) {
        FRESULT res = f_readdir(&dir, &fileInfo);
        if(res != FR_OK || fileInfo.fname[0] == '\0' || index >= MAX_FILE_LIST_LENGTH) 
            break;
        
        strcpy(ctx.filenames[index], fileInfo.fname);
        toLower(ctx.filenames[index]);
        
        ++index;
    }
}