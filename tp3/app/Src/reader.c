#include <stdbool.h>
#include "reader.h"

#include <stdio.h>

#include "dto.h"
#include "uart.h"

typedef struct {
    QueueHandle_t hqueue;
} reader_t;

static reader_t reader;
static char msgRead[256];

static void processMessage(void * _) {
    char mander;
    size_t msgLen = 0;
    while (true) {
        bool done = false;
        bool append = false;
        while (!done) {
            uart_receive(&mander, 1);
            if (mander == '{') {
                append = true;
                msgLen = 0;
                msgRead[0] = 0x00;
            }
            if (append) {
                msgRead[msgLen++] = mander;
            }
            if (append && mander == '}') {
                append = false;
                done = true;
            }
        }
        request_data_t msg = { 0 };
        msgRead[msgLen] = 0x00;
        int parsed = sscanf(msgRead,
                    "{type:%*hu,id:%hu,priority:%hu}",
                    &msg.request_id,
                    &msg.priority);

        if (parsed != 3) {
            continue; // invalid message
        }
        if (pdPASS != xQueueSend(reader.hqueue, &msg, portMAX_DELAY)) {
            request_data_t msgDeleted;
            xQueueReceive(reader.hqueue, &msgDeleted, portMAX_DELAY);
            if (pdPASS != xQueueSend(reader.hqueue, &msg, portMAX_DELAY)) {
                // Critical error
            }
        }
    }
}
app_err_t readerInit(QueueHandle_t msgQueue) {
    reader.hqueue = msgQueue;
    const bool ok = pdPASS == xTaskCreate(processMessage, "reader", 128, NULL, tskIDLE_PRIORITY, NULL);
    return ok ? APP_OK : APP_ERR_INTERNAL;
}