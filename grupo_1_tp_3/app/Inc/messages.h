#ifndef INC_MESSAGES_H_
#define INC_MESSAGES_H_

#include "error.h"
#include <stdbool.h>

// TODO: check this number
#define MSG_ERR_UNKNOWN 	(ERR_BASE_MSG + 1)

#define MAX_TICK_MSG_LEN 256

typedef enum {
	TICK_MSG,
	REQUEST_MSG,
	RESPONSE_MSG,
} msg_type_t;


void build_tick_message(char* msg, uint32_t stamp_ms);

bool is_valid_message_type(uint8_t msg_type);

#endif /* INC_MESSAGES_H_ */
