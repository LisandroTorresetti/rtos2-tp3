/********************** inclusions *******************************************/


#include "tick_task.h"
#include "messages.h"
#include "main.h"
#include "uart.h"
#include <stdio.h>
#include <stdint.h>
#include "cmsis_os.h"


/********************** macros and definitions *******************************/

#define TICK_TASK_PERIOD_MS 100

/********************** external functions definition ************************/

void task_ticks(void* argument) {
	TickType_t last_wake_time = xTaskGetTickCount();
	const TickType_t frequency = pdMS_TO_TICKS(TICK_TASK_PERIOD_MS);

	while(1) {
		uint32_t ticks = xTaskGetTickCount();
		char tick_msg[MAX_TICK_MSG_LEN] = {0};
		build_tick_message(tick_msg, ticks);

		app_err_t err = uart_send(tick_msg, MAX_TICK_MSG_LEN);
		if (err != APP_OK) {
			// TODO: log error message
		}

		vTaskDelayUntil(&last_wake_time, frequency);
  }
}

/********************** end of file ******************************************/
