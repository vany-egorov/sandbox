#include "msg-in.h"


void msg_in_sprint_json(MsgIn *it, char *buf, size_t bufsz) {
	snprintf(buf, bufsz, "{"
		"\"id\": %zd"
		", \"buf\": \"%s\""
		"}",
		it->id,
		it->buf
	);
}

void msg_in_print_json(MsgIn *it) {
	char buf[300];
	msg_in_sprint_json(it, buf, sizeof(buf));
	printf("%s\n", buf);
}
