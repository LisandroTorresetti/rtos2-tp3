#ifndef GRUPO_1_TP_3_READER_H
#define GRUPO_1_TP_3_READER_H
#include "error.h"
#include "cmsis_os.h"
app_err_t readerInit(QueueHandle_t msgQueue);

#endif //GRUPO_1_TP_3_READER_H