#ifndef __MSG_IN_H__
#define __MSG_IN_H__


#include <stdint.h>  /* uint64_t */
#include <stddef.h>  /* size_t */
#include <stdio.h>   /* printf, sprintf */

typedef struct msg_in_s MsgIn;


struct msg_in_s {
	uint64_t id;
	char buf[255];
};


void msg_in_sprint_json(MsgIn *it, char *buf, size_t bufsz);
void msg_in_print_json(MsgIn *it);


#endif /* __MSG_IN_H__ */
