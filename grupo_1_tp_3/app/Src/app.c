#include "app.h"

#include "dispatcher.h"
#include "hash.h"
#include "heap.h"
#include "main.h"
#include "processor.h"
#include "reader.h"
#include "tick_task.h"
#include "uart.h"
#include "dto.h"
#define QUEUE_MESSAGE_SIZE (sizeof(request_data_t)) // Define msg size
#define QUEUE_LENGTH 50
#define useReader 1
#define useDispatcher 1
#define useProcessor 1

static SemaphoreHandle_t writeSem;
static SemaphoreHandle_t controllerSem;
static Heap heap;
static QueueHandle_t hqueue
app_err_t app_init() {
	if (APP_OK != uart_init()) {
		Error_Handler();
	}
	hqueue = xQueueCreate(QUEUE_LENGTH, QUEUE_MESSAGE_SIZE);
	if (hqueue == NULL) {
		Error_Handler();
	}
	writeSem = xSemaphoreCreateBinary(); // For tik tak + processor purposes
	controllerSem = xSemaphoreCreateBinary(); // For dispatcher + processor purposes
	if (writeSem == NULL || controllerSem == NULL) {
		Error_Handler();
	}
	if (pdPASS != xSemaphoreGive(writeSem)) {
		Error_Handler();
	}
	if (pdPASS != xSemaphoreGive(controllerSem)) {
			Error_Handler();
		}
	heapInit(&heap);
	if (APP_OK != hashInit()) {
		Error_Handler();
	}
#if useReader
	if (APP_OK != readerInit(hqueue)) {
		Error_Handler();
	}
#endif
#if useDispatcher

	if (APP_OK != dispatcherInit(&heap, hqueue, controllerSem)) {
		Error_Handler();
	}
#endif
#if useProcessor
	if (APP_OK != processorInit(&heap, controllerSem, writeSem)) {
		Error_Handler();
	}
#endif
	return tickTaskInit(writeSem);
}

