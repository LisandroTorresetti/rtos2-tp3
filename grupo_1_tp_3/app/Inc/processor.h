#ifndef GRUPO_1_TP_3_PROCESSOR_H
#define GRUPO_1_TP_3_PROCESSOR_H
#include "error.h"
#include "heap.h"
#include "cmsis_os.h"

app_err_t processorInit(Heap* heap, SemaphoreHandle_t sem, SemaphoreHandle_t sendingSem);
#endif //GRUPO_1_TP_3_PROCESSOR_H