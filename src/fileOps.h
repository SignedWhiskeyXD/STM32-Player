#ifndef FILEOPS_H
#define FILEOPS_H

#include "stm32f10x.h"
#include "myError.h"

#define MAX_FILE_LIST_LENGTH 16

MYERROR initSD();

void loadFiles();

#endif