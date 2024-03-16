#include "daemon_tasks.h"

#include "button.h"
#include "display.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"

static const char* taskNameKeyScan  = "TaskKeyScan";
static const char* taskNameScreen   = "TaskScreen";
static const char* taskNameCreation = "TaskCreation";

static TaskHandle_t taskCreationHandler;
static TaskHandle_t taskKeyScanHandler;
static TaskHandle_t taskScreenHandler;

void taskKeyScan()
{
    while (1) {
        scanKeys();
        vTaskDelay(10);
    }
}

void taskScreenRefresh()
{
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        onScreenRefresh();
    }
}

void taskCreation()
{
    taskENTER_CRITICAL();

    xTaskCreate(taskKeyScan, taskNameKeyScan, 512, NULL, 2, &taskKeyScanHandler);
    xTaskCreate(taskScreenRefresh, taskNameScreen, 512, NULL, 3, &taskScreenHandler);

    notifyScreenRefresh();

    vTaskDelete(taskCreationHandler);

    taskEXIT_CRITICAL();
}

void launchDaemonTasks()
{
    xTaskCreate(taskCreation, taskNameCreation, 512, NULL, 2, &taskCreationHandler);

    vTaskStartScheduler();
}

void notifyScreenRefresh()
{
    xTaskNotifyGive(taskScreenHandler);
}