#ifndef __ENC_H__
#define __ENC_H__


#include <stdint.h>   /* uint16_t */
#include <pthread.h>  /* pthread_t */

#include "./chan.h"


typedef struct enc_s ENC;

struct enc_s {
	uint16_t w;
	uint16_t h;
	uint16_t b;

	pthread_t thread;

	Chan *chan_i;
	Chan *chan_o;
};


int enc_new(ENC **out);
int enc_init(ENC *it, size_t cap_chan_i, size_t cap_chan_o);
int enc_del(ENC *it);


#endif /* __ENC_H__ */
