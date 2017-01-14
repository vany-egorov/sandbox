#ifndef __MSGS_H__
#define __MSGS_H__


#include <stdio.h>   /* printf */
#include <stddef.h>  /* size_t */
#include <stdlib.h>  /* malloc, calloc, free */


typedef struct msgs Msgs;


struct msgs {
	size_t len, cap,
	       start, finish;

	size_t elsz;
	void *els;
};


int  msgs_new(Msgs **out, size_t cap, size_t elsz);
int  msgs_get_available(Msgs *it, void **out);
int  msgs_got_available(Msgs *it);
int  msgs_is_empty(Msgs *it);
int  msgs_first(Msgs *it, void **out);
void msgs_del(Msgs *it);


#endif /* __MSGS_H__ */
