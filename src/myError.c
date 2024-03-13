#include "myError.h"

MyError lastError = OPERATION_SUCCESS;

MyError getLastError()
{
    return lastError;
}

void setLastError(MyError error)
{
    lastError = error;
}