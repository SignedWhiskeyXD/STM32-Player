#include "file_ops.h"

#include <stdbool.h>
#include <string.h>

File_State ctx;

static void toLower(char* filename)
{
    for (uint8_t i = 0; i < MAX_LFN_LENGTH && filename[i] != '\0'; ++i) {
        if (filename[i] >= 65 && filename[i] <= 90) filename[i] += 32;
    }
}

static bool isMusicFile(TCHAR* filename)
{
    static const char*   extensions[2] = {"mp3", "wav"};
    static const uint8_t tableSize     = sizeof(extensions) / sizeof(extensions[0]);

    char* endPos = strstr(filename, ".");
    if (!endPos || endPos - filename > 10) return 0;

    for (uint8_t i = 0; i < tableSize; ++i) {
        if (strncmp(endPos + 1, extensions[i], 3) == 0) return true;
    }
    return false;
}

File_State* useFileState()
{
    return &ctx;
}

MyError initSD()
{
    FATFS   fs;
    FRESULT res = f_mount(&fs, "0:", 1);
    if (res != FR_OK) return SD_FATFS_MOUNT_ERROR;
    return OPERATION_SUCCESS;
}

void loadFiles()
{
    DIR     dir;
    FRESULT res = f_opendir(&dir, "0:/");

    FILINFO fileInfo;
    fileInfo.lfname = ctx.filenames[0];
    fileInfo.lfsize = MAX_LFN_LENGTH;

    uint8_t idx = 0;
    while (idx < MAX_FILE_LIST_LENGTH) {
        res = f_readdir(&dir, &fileInfo);
        if (res != FR_OK || fileInfo.fname[0] == '\0') break;

        toLower(fileInfo.fname);
        if (!isMusicFile(fileInfo.fname)) continue;

        // 如果长文件名不可用，则以短文件名回退，并且转小写
        if (ctx.filenames[idx][0] == '\0') strcpy(ctx.filenames[idx], fileInfo.fname);

        fileInfo.lfname = ctx.filenames[++idx];
    }

    f_closedir(&dir);
    ctx.totalFiles = idx;
    ctx.nowPlaying = idx;
}

void moveFilePointer(int8_t delta)
{
    if (delta == 0) return;

    const int8_t newPos = ctx.filenameBase + ctx.offset + delta;

    if (newPos >= ctx.totalFiles || newPos < 0) return;

    if (delta > 0) {
        if (ctx.offset < DIR_MAX_LINES - 1) {
            ctx.offset++;
        } else if (ctx.filenameBase + DIR_MAX_LINES < MAX_FILE_LIST_LENGTH &&
                   ctx.filenames[ctx.filenameBase + DIR_MAX_LINES][0] != '\0') {
            ctx.filenameBase++;
        }
    } else {
        if (ctx.offset > 0)
            ctx.offset--;
        else if (ctx.filenameBase > 0) {
            ctx.filenameBase--;
        }
    }
}