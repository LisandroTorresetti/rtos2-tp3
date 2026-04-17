#ifndef UART_INC_UART_H_
#define UART_INC_UART_H_

#include <stdint.h>
#include "error.h"

#define UART_ERR_INIT   (ERR_BASE_UART + 1)
#define UART_ERR_TX     (ERR_BASE_UART + 2)
#define UART_ERR_RX     (ERR_BASE_UART + 3)

app_err_t uart_init();

app_err_t uart_send(char* pstring, uint16_t size);

app_err_t uart_receive(char* pstring, uint16_t size);

#endif /* UART_INC_UART_H_ */
