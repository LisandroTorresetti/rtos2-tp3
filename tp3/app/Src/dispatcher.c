#include "dispatcher.h"

typedef struct {
    QueueHandle_t hqueue;
    Heap* heap;
} dispatcher_t;
dispatcher_t dispatcher;

void processMessage(void) {
    void* msg;
    if (pdPASS == xQueueReceive(dispatcher.hqueue, &msg, portMAX_DELAY)) {
        
    }
}
app_err_t dispatcherInit(Heap* heap, QueueHandle_t msgQueue) {
    dispatcher.heap = heap;
    dispatcher.hqueue = msgQueue;
    return APP_OK;
}