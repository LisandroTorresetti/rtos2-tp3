#ifndef INC_ERROR_H_
#define INC_ERROR_H_

#include <stdint.h>

typedef int32_t app_err_t;

#define APP_OK 				 0
#define APP_FAIL        	-1
#define APP_ERR_INTERNAL 	-2
#define APP_ERR_UNKNOWN 	-3

#define ERR_BASE_UART       0x1000
#define ERR_BASE_MSG 		0x2000

#endif /* INC_ERROR_H_ */
