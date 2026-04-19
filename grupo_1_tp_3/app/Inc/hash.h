#ifndef GRUPO_1_TP_3_HASH_H
#define GRUPO_1_TP_3_HASH_H
#include <stdint.h>

#include "cmsis_os.h"
#include "uthash.h"
#include "error.h"

#define REQUEST_ID_AMOUNT 8
#define MAX_ITEMS 32

typedef struct {
    int priority;
    uint16_t requestId[REQUEST_ID_AMOUNT];
#if MAX_ITEMS < 255
    uint8_t count;
#else
    uint16_t count;
#endif
    xSemaphoreHandle mutex;
    UT_hash_handle hh;
} PriorityItem;

// API
void hashAdd(int priority, uint16_t requestId);
PriorityItem* hashFind(int priority);
void hashDeleteItem(int priority);
app_err_t hashInit(void);
#endif //GRUPO_1_TP_3_HASH_H