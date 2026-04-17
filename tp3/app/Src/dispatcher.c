#include "dispatcher.h"
#include "dto.h"
#include "hash.h"

typedef struct {
    QueueHandle_t hqueue;
    Heap* heap;
} dispatcher_t;
dispatcher_t dispatcher;

void processMessage(void) {
    request_data_t msg;
    if (pdPASS == xQueueReceive(dispatcher.hqueue, &msg, portMAX_DELAY)) {
        hashAdd(msg.priority, msg.request_id);
        heapPush(dispatcher.heap, msg.request_id);
    }
}
app_err_t dispatcherInit(Heap* heap, QueueHandle_t msgQueue) {
    dispatcher.heap = heap;
    dispatcher.hqueue = msgQueue;
    return APP_OK;
}