#include "daemon_tasks.h"

#include "button.h"
#include "display.h"
#include "lvgl/lvgl.h"
#include "rtos/FreeRTOS.h"
#include "rtos/task.h"

static const char* taskNameKeyScan  = "TaskKeyScan";
static const char* taskNameScreen   = "TaskScreen";
static const char* taskNameCreation = "TaskCreation";
static const char* taskNameLVGL     = "TaskLVGL";

static TaskHandle_t taskCreationHandler;
static TaskHandle_t taskKeyScanHandler;
static TaskHandle_t taskScreenHandler;
static TaskHandle_t taskLVGLHandler;

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

void taskTFT()
{
    while (1) {
        const uint32_t delay = lv_timer_handler();
        vTaskDelay(delay);
    }
}

void taskCreation()
{
    taskENTER_CRITICAL();

    xTaskCreate(taskKeyScan, taskNameKeyScan, 128, NULL, 2, &taskKeyScanHandler);
    xTaskCreate(taskScreenRefresh, taskNameScreen, 128, NULL, 3, &taskScreenHandler);

#ifdef USE_LVGL
    xTaskCreate(taskTFT, taskNameLVGL, 1024, NULL, 4, &taskLVGLHandler);
#endif

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