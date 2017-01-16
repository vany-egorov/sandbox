#ifndef __ENCS_H__
#define __ENCS_H__


#include <stddef.h>  /* size_t */
#include <stdlib.h>  /* malloc, calloc, free */

#include "./enc.h"


typedef struct encs_c ENCs;


struct encs_c {
	size_t len;
	ENC **els;
};


int encs_new(ENCs **out);
int encs_push(ENCs *it, ENC *el);
int encs_del(ENCs **out);


#endif /* __ENCS_H__ */
