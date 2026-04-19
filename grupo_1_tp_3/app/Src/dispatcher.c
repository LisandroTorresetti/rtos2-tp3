#include "dispatcher.h"
#include "dto.h"
#include "hash.h"

typedef struct {
    QueueHandle_t hqueue;
    Heap* heap;
    SemaphoreHandle_t sem;
} dispatcher_t;

static dispatcher_t dispatcher;

static void processMessage(void * _) {
    while (true) {
        request_data_t msg;
        if (pdPASS == xQueueReceive(dispatcher.hqueue, &msg, portMAX_DELAY)) {
            xSemaphoreTake(dispatcher.sem, portMAX_DELAY);
            hashAdd(msg.priority, msg.request_id);
            heapPush(dispatcher.heap, msg.priority);
            xSemaphoreGive(dispatcher.sem);
        }
    }
}
app_err_t dispatcherInit(Heap* heap, QueueHandle_t msgQueue, SemaphoreHandle_t controllerSem) {
    dispatcher.heap = heap;
    dispatcher.hqueue = msgQueue;
    dispatcher.sem = controllerSem;
    const bool ok = pdPASS == xTaskCreate(processMessage, "dispatcher", 512, NULL, tskIDLE_PRIORITY, NULL);
    return ok ? APP_OK : APP_ERR_INTERNAL;
}
