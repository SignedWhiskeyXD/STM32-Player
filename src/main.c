#include "myError.h"
#include "states/states.h"
#include "button.h"
#include "display.h"
#include "vs1053/VS1053.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"
#include "daemon_tasks.h"

void initPlayer()
{
    initScreen();
    initKeys();

    MyError error = initSD();
    if(error != OPERATION_SUCCESS){
        setLastError(error);
        setGlobalState(PLAYER_ERROR);
        return;
    }
    loadFiles();
    
    VS_Init();
    VS_HD_Reset();
	VS_Soft_Reset();
    
    setGlobalState(BROWSING_MENU);
}

int main()
{
    initPlayer();

    launchDaemonTasks();

    while (1);
}