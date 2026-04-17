#include "app.h"
#include "tick_task.h"

app_err_t app_init() {



	task_ticks(0);
	return APP_OK;
}

