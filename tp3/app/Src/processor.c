#include "processor.h"

#include <stdbool.h>
#include <stdio.h>

#include "cmsis_os.h"
#include "hash.h"
#include "uart.h"
#define TIMEOUT_BETWEEN_EMPTY_MSGS_MS (1000 / portTICK_PERIOD_MS)
#define ZERO_AMOUNT_FRAME 256
typedef struct {
    Heap* heap;
    SemaphoreHandle_t sem;
    SemaphoreHandle_t sendingSem;
} processor_t;
static processor_t processor;

static char buffer[40 + ZERO_AMOUNT_FRAME];

static size_t createMsg(int requestId) {
    buffer[0] = 0x00;
    size_t str_len = sprintf(buffer, "{type:2,requestId:%d", requestId);
    char *pwrite = buffer;
    pwrite += str_len;
    memset(pwrite , '0', 256);
    pwrite += 256;
    sprintf(pwrite, "}");
    return str_len + 256 + 1;
}

static void processMessage(void* _) {
    int msg;
    while (true) {
        xSemaphoreTake(processor.sendingSem, portMAX_DELAY);
        xSemaphoreTake(processor.sem, portMAX_DELAY);
        if (!heapPop(processor.heap, &msg)) {
            xSemaphoreGive(processor.sem);
            xSemaphoreGive(processor.sendingSem);
            vTaskDelay(TIMEOUT_BETWEEN_EMPTY_MSGS_MS);
            continue;
        }
        xSemaphoreGive(processor.sem);
        PriorityItem* item = hashFind(msg);
        int requestId = item->requestId[0];
        if (item->count > 1) {
            for (int i = 1; i < item->count; i++) {
                item->requestId[i - 1] = item->requestId[i];
            }
        } else {
            hashDeleteItem(msg);
        }
        size_t size = createMsg(requestId);
        uart_send(buffer, size);
        xSemaphoreGive(processor.sendingSem);
    }
}

app_err_t processorInit(Heap* heap, SemaphoreHandle_t sem, SemaphoreHandle_t sendingSem) {
    processor.heap = heap;
    processor.sem = sem;
    processor.sendingSem = sendingSem;
    const bool ok = pdPASS == xTaskCreate(processMessage, "processor", 128, NULL, tskIDLE_PRIORITY, NULL);
    return ok ? APP_OK : APP_ERR_INTERNAL;
}