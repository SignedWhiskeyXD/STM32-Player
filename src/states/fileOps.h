#ifndef FILEOPS_H
#define FILEOPS_H

#include "stm32f10x.h"
#include "myError.h"
#include <stdint.h>

#define MAX_FILE_LIST_LENGTH 16

typedef struct {
    
    // 当前已读取的目录文件名
    char filenames[MAX_FILE_LIST_LENGTH][13];

    uint8_t totalFiles;

    // 目录项显示的起始索引
    uint8_t filenameBase;

    uint8_t offset;
} File_State;

File_State* useFileState();

MYERROR initSD();

void loadFiles();

void moveFilePointer(int8_t delta);

#endif