#ifndef FILEOPS_H
#define FILEOPS_H

#include "stm32f10x.h"
#include "myError.h"

#define MAX_FILE_LIST_LENGTH 16

typedef struct {
    
    // 当前已读取的目录文件名
    char filenames[MAX_FILE_LIST_LENGTH][12];

    // 目录项显示的起始索引
    uint8_t filenameBase;
} File_State;

File_State* useFileState();

MYERROR initSD();

void loadFiles();

#endif