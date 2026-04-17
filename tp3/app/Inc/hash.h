#ifndef GRUPO_1_TP_3_HASH_H
#define GRUPO_1_TP_3_HASH_H
#include <stdint.h>
#include "uthash.h"

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

    UT_hash_handle hh;
} PriorityItem;

// API
void phash_add(int priority, uint16_t requestId);
PriorityItem* phash_find(int priority);
void phash_delete(int priority);
void phash_clear(void);
#endif //GRUPO_1_TP_3_HASH_H