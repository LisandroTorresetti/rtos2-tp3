/********************** inclusions *******************************************/


#include "tick_task.h"
#include "messages.h"
#include "uart.h"
#include <stdio.h>
#include <stdint.h>
#include "cmsis_os.h"


/********************** macros and definitions *******************************/

#define TICK_TASK_PERIOD_MS (100)
#define MAX_TIME_PROCESSING_MESSAGE_MS (27)
#define INITIAL_TIME_SLEEP (TICK_TASK_PERIOD_MS - MAX_TIME_PROCESSING_MESSAGE_MS)
typedef struct {
  SemaphoreHandle_t sem;
} task_ticks_t;
static task_ticks_t tasks;
/********************** external functions definition ************************/

static void sendTickMessage(void) {
	uint32_t ticks = xTaskGetTickCount();
	char tick_msg[MAX_TICK_MSG_LEN] = {0};
	build_tick_message(tick_msg, ticks);

	app_err_t err = uart_send(tick_msg, MAX_TICK_MSG_LEN);
	if (err != APP_OK) {
		// TODO: log error message
	}
}

static void task_ticks(void* _) {
	TickType_t last_wake_time = xTaskGetTickCount();
	const TickType_t frequency = pdMS_TO_TICKS(TICK_TASK_PERIOD_MS);

	sendTickMessage();
	while(1) {
		vTaskDelayUntil(&last_wake_time, INITIAL_TIME_SLEEP);
		xSemaphoreTake(tasks.sem, portMAX_DELAY);
		TickType_t currentTime = xTaskGetTickCount();
		vTaskDelayUntil(&last_wake_time, MAX_TIME_PROCESSING_MESSAGE_MS - (currentTime - last_wake_time));
		sendTickMessage();
		xSemaphoreGive(tasks.sem);
  }
}

app_err_t tickTaskInit(SemaphoreHandle_t sem) {
	tasks.sem = sem;
	if (pdPASS != xTaskCreate(task_ticks, "tick_task", 128, NULL, tskIDLE_PRIORITY, NULL)) {
		return APP_ERR_INTERNAL;
	}
	return APP_OK;
}

/********************** end of file ******************************************/
