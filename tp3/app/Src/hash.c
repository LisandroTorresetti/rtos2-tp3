#include "hash.h"

#include <stdbool.h>
#include <string.h>

static PriorityItem pool[MAX_ITEMS];
static bool used[MAX_ITEMS];

static PriorityItem *table = NULL;

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

void phash_add(int priority, uint16_t requestId) {
    PriorityItem *item;

    HASH_FIND_INT(table, &priority, item);

    if (item == NULL) {
        item = sallocItem();
        if (item == NULL) return; // ToDO handle scenario for item not being able to be inserted in the hash

        item->priority = priority;
        HASH_ADD_INT(table, priority, item);
    }

    if (item->count < REQUEST_ID_AMOUNT) {
        item->requestId[item->count++] = requestId;
    } else {
        for (int i = 1; i < REQUEST_ID_AMOUNT; i++) {
            item->requestId[i - 1] = item->requestId[i];
        }
        item->requestId[REQUEST_ID_AMOUNT - 1] = requestId;
    }
}

PriorityItem* phash_find(const int priority) {
    PriorityItem *item;
    HASH_FIND_INT(table, &priority, item);
    return item;
}

void phash_delete(const int priority) {
    PriorityItem *item;

    HASH_FIND_INT(table, &priority, item);

    if (item != NULL) {
        HASH_DEL(table, item);
        freeItem(item);
    }
}