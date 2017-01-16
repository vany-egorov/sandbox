#ifndef __ENC_H__
#define __ENC_H__


#include <stddef.h>   /* size_t */
#include <pthread.h>  /* pthread_t */

#include <x265.h>  /* x265_encoder */

#include "./chan.h"
#include "./profile.h"
#include "./msg-i.h"
#include "./msg-o.h"


typedef struct enc_s ENC;
typedef struct enc_param_s ENCParam;


struct enc_param_s {
	Profile profile;

	size_t cap_chan_i;
	size_t cap_chan_o;
};

struct enc_s {
	ENCParam param;

	Chan *chan_i;
	Chan *chan_o;

	pthread_t thread;

	x265_encoder *h265_encoder;
	x265_picture *h265_pic_in;
	x265_picture *h265_pic_out;
};


int enc_new(ENC **out, ENCParam *param);
int enc_go(ENC *it);
int enc_del(ENC **out);


#endif /* __ENC_H__ */
