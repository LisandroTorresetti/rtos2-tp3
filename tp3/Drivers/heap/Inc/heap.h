#ifndef GRUPO_1_TP_3_HEAP_H
#define GRUPO_1_TP_3_HEAP_H
#define HEAP_MAX 1000
// heap of numbers because we will manage priorities only
typedef struct {
    int data[HEAP_MAX];
    int size;
} Heap;

void heapInit(Heap *h);
int heapPush(Heap *h, int value);   // returns 0 if full
int heapPop(Heap *h, int *out);     // returns 0 if empty
int heapPeek(const Heap *h, int *out);    // returns 0 if empty

#endif //GRUPO_1_TP_3_HEAP_H