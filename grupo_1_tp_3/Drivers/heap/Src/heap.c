#include "heap.h"

static void swap(int *a, int *b) {
    const int t = *a;
    *a = *b;
    *b = t;
}

void heapInit(Heap *h) {
    h->size = 0;
}

bool heapPush(Heap *h, const int value) {
    if (h->size >= HEAP_MAX) return false;

    int i = h->size++;
    h->data[i] = value;

    while (i > 0) {
        const int parent = (i - 1) / 2;

        if (h->data[parent] >= h->data[i]) break;

        swap(&h->data[parent], &h->data[i]);
        i = parent;
    }

    return true;
}

bool heapPop(Heap *h, int *out) {
    if (h->size == 0) return false;

    *out = h->data[0];
    h->data[0] = h->data[--h->size];

    int i = 0;

    while (true) {
        const int left = 2*i + 1;
        const int right = 2*i + 2;
        int largest = i;

        if (left < h->size && h->data[left] > h->data[largest])
            largest = left;

        if (right < h->size && h->data[right] > h->data[largest])
            largest = right;

        if (largest == i) break;

        swap(&h->data[i], &h->data[largest]);
        i = largest;
    }

    return true;
}

bool heapPeek(const Heap *h, int *out) {
    if (h->size == 0) return false;

    *out = h->data[0];
    return true;
}