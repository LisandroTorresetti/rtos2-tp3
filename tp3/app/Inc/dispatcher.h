#ifndef GRUPO_1_TP_3_DISPATCHER_H
#define GRUPO_1_TP_3_DISPATCHER_H
#include "error.h"
#include "heap.h"
#include "cmsis_os.h"

app_err_t dispatcherInit(Heap* heap, QueueHandle_t msgQueue, SemaphoreHandle_t controllerSem);
#endif //GRUPO_1_TP_3_DISPATCHER_H