#ifndef __MSG_O_H__
#define __MSG_O_H__


#include <stdint.h>  /* uint8_t, uint32_t */

#include "./profile.h"
#include "./enc-status.h"


typedef struct msg_o_s MsgO;


struct msg_o_s {
	ENCStatus status;

	uint32_t nal_type;
	uint8_t *nal_payload;
	uint32_t nal_sz; /* size in bytes */

	Profile *profile;
};


void msg_o_clear(MsgO *it);


#endif /* __MSG_O_H__ */
