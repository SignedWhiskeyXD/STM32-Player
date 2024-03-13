#ifndef MY_ERROR_H
#define MY_ERROR_H

typedef enum {
    OPERATION_SUCCESS,
    SD_FATFS_MOUNT_ERROR
} MyError;

MyError getLastError();

void setLastError(MyError error);

#endif