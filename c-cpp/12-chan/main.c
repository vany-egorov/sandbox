#include <stdio.h>     /* printf */
#include <errno.h>     /* errno */
#include <string.h>    /* strncpy */
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */

#include "./chan.h"
#include "./msgs.h"
#include "./msg-in.h"
#include "./msg-out.h"


void* read_worker_do(void *args) {
	printf("[%p]\n", args);
	return NULL;
}


int main (int argc, char *argv[]) {
	int ret = EX_OK;

	Msgs *msgs_in = NULL,
	     *msgs_out = NULL;
	Chan *chan_it = NULL,
	     *chan_out = NULL;

	msgs_new(&msgs_in, 1, sizeof(MsgIn));
	msgs_new(&msgs_out, 1, sizeof(MsgOut));

	chan_new(&chan_it, 10, sizeof(void *));
	chan_new(&chan_out, 10, sizeof(void *));

	MsgIn *msg_in1 = NULL;
	msgs_get_available(msgs_in, (void*)&msg_in1);

	MsgIn *msg_in2 = NULL;
	msgs_get_available(msgs_in, (void*)&msg_in2);

	char *profiles[] = {
		"384x216@204",
		"640x360@504",
		"1024x576@1272",
		"1280x720@2072",
		NULL,
	};
	char **profile = NULL;

	for (profile=profiles; *profile; profile++) {
		pthread_t t = { 0 };
		if (pthread_create(&t, NULL, read_worker_do, NULL)) {
			fprintf(stderr, "pthread-create error: \"%s\"\n", strerror(errno));
			ret = EX_SOFTWARE; goto cleanup;
		} else
			printf("[read-worker @ %p] OK \n", &t);
	}

	// msg_in1->id = 1;
	// strncpy(msg_in1->buf, "msg-in-1", sizeof(msg_in1->buf)-1);

	// msg_in2->id = 2;
	// strncpy(msg_in2->buf, "msg-in-2", sizeof(msg_in2->buf)-1);

	// msg_in_print_json(msg_in1);
	// msg_in_print_json(msg_in2);

cleanup:
	return ret;
}
