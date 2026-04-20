#ifndef GRUPO_1_TP_3_HEAP_H
#define GRUPO_1_TP_3_HEAP_H
#define HEAP_MAX 1000
#include <stdbool.h>
// heap of numbers because we will manage priorities only
typedef struct {
    int data[HEAP_MAX];
    int size;
} Heap;

void heapInit(Heap *h);
bool heapPush(Heap *h, int value);
bool heapPop(Heap *h, int *out);
bool heapPeek(const Heap *h, int *out);

#endif //GRUPO_1_TP_3_HEAP_H