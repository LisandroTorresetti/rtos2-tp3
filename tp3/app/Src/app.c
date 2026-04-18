#include "app.h"

#include "dispatcher.h"
#include "hash.h"
#include "heap.h"
#include "main.h"
#include "processor.h"
#include "tick_task.h"

#define QUEUE_MESSAGE_SIZE 1024 // Define msg size
#define QUEUE_LENGTH 50

app_err_t app_init() {
	QueueHandle_t hqueue = xQueueCreate(QUEUE_LENGTH, QUEUE_MESSAGE_SIZE);
	if (hqueue == NULL) {
		Error_Handler();
	}
	const SemaphoreHandle_t writeSem = xSemaphoreCreateBinary(); // For tik tak + processor purposes
	const SemaphoreHandle_t controllerSem = xSemaphoreCreateBinary(); // For dispatcher + processor purposes
	if (writeSem == NULL || controllerSem == NULL) {
		Error_Handler();
	}
	Heap heap;
	heapInit(&heap);
	if (APP_OK != hashInit()) {
		Error_Handler();
	}
	if (APP_OK != dispatcherInit(&heap, hqueue, controllerSem)) {
		Error_Handler();
	}
	if (APP_OK != processorInit(&heap, controllerSem, writeSem)) {
		Error_Handler();
	}
	task_ticks(0);
	return APP_OK;
}

