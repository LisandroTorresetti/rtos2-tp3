#include "messages.h"
#include <stdio.h>

static char* TICK_MSG_TEMPLATE = "{id:%d,stamp:%d}";

bool is_valid_message_type(uint8_t msg_type) {
	return msg_type >= TICK_MSG && msg_type <= RESPONSE_MSG;
}

void build_tick_message(char* msg, const uint32_t stamp_ms) {
	snprintf(msg, MAX_TICK_MSG_LEN, TICK_MSG_TEMPLATE, TICK_MSG, stamp_ms);
}
