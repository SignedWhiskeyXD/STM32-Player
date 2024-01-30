#include "myError.h"

MYERROR lastError = OPERATION_SUCCESS;

MYERROR getLastError()
{
    return lastError;
}

void setLastError(MYERROR error)
{
    lastError = error;
}