#ifndef INC_TICK_TASK_H_
#define INC_TICK_TASK_H_

#include "error.h"
#include "cmsis_os.h"
app_err_t tickTaskInit(SemaphoreHandle_t sem);

#endif /* INC_TICK_TASK_H_ */
