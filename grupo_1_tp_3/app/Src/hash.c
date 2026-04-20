#include "hash.h"

#include <stdbool.h>
#include <string.h>

static PriorityItem pool[MAX_ITEMS];
static bool used[MAX_ITEMS];

static PriorityItem *table = NULL; // This will be managed later by utash, null table = handled there
static xSemaphoreHandle writeDeleteMutex = NULL;

static PriorityItem* sallocItem(void) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!used[i]) {
            used[i] = true;
            return &pool[i];
        }
    }
    return NULL; // no space left
}

static void freeItem(const PriorityItem *item) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (&pool[i] == item) {
            used[i] = false;
            return;
        }
    }
}

void hashAdd(int priority, uint16_t requestId) {
    xSemaphoreTake(writeDeleteMutex, portMAX_DELAY);
    PriorityItem *item;

    HASH_FIND_INT(table, &priority, item);

    if (item == NULL) {
        item = sallocItem();
        if (item == NULL) {
        	xSemaphoreGive(writeDeleteMutex);
        	return; // ToDO handle scenario for item not being able to be inserted in the hash
        }
        memset(item, 0, sizeof(PriorityItem));
        item->mutex = xSemaphoreCreateBinary();
        xSemaphoreGive(item->mutex);
        if (item->mutex == NULL) {
        	xSemaphoreGive(writeDeleteMutex);
        	return; // ToDO handle scenario for item not being able to be inserted in the hash
        }

        item->priority = priority;
        item->count = 0;
        HASH_ADD_INT(table, priority, item);
    }
    xSemaphoreTake(item->mutex, portMAX_DELAY);

    if (item->count < REQUEST_ID_AMOUNT) {
        item->requestId[item->count++] = requestId;
    } else {
        for (int i = 1; i < REQUEST_ID_AMOUNT; i++) {
            item->requestId[i - 1] = item->requestId[i];
        }
        item->requestId[REQUEST_ID_AMOUNT - 1] = requestId;
    }
    xSemaphoreGive(item->mutex);
    xSemaphoreGive(writeDeleteMutex);
}

PriorityItem* hashFind(int priority) {
    PriorityItem *item;
    HASH_FIND_INT(table, &priority, item);
    return item;
}

void hashDeleteItem(const int priority) {
    xSemaphoreTake(writeDeleteMutex, portMAX_DELAY);
    PriorityItem *item;

    HASH_FIND_INT(table, &priority, item);

    // We won't delete items that have sub items inside (To avoid delete after write)
    if (item != NULL && item->count == 0) {
        SemaphoreHandle_t mutex = item->mutex;
        xSemaphoreTake(mutex, portMAX_DELAY);
        HASH_DEL(table, item);
        freeItem(item);
        xSemaphoreGive(mutex);
        vSemaphoreDelete(mutex); // We have to release before deleting for rtos purposes
    }
    xSemaphoreGive(writeDeleteMutex);
}

app_err_t hashInit(void) {
    writeDeleteMutex = xSemaphoreCreateBinary();
    if (writeDeleteMutex == NULL) {
        return APP_ERR_INTERNAL;
    }
    xSemaphoreGive(writeDeleteMutex);
    return APP_OK;
}
