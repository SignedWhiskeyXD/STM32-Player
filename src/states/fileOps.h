#ifndef FILEOPS_H
#define FILEOPS_H

#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "FatFs/ff.h"
#include "states.h"

#define MAX_FILE_LIST_LENGTH 32

#define MAX_LFN_LENGTH 64

#define DIR_MAX_LINES 3

typedef struct {
    
    // 当前已读取的目录文件名
    TCHAR filenames[MAX_FILE_LIST_LENGTH][MAX_LFN_LENGTH];

    // 所有已加载的文件总数
    uint8_t totalFiles;

    // 目录项显示的起始索引
    uint8_t filenameBase;

    // 当前选中的相对行数
    uint8_t offset;

    // 当前播放曲目的索引
    uint8_t nowPlaying;

    // 当前曲目是否被暂停
    uint8_t paused;
} File_State;

File_State* useFileState();

MyError initSD();

void loadFiles();

void moveFilePointer(int8_t delta);

#endif