#include "msg-o.h"


void msg_o_clear(MsgO *it) {
	it->status = ENC_STATUS_MORE;

	it->nal_type = 0;
	it->nal_payload = NULL;
	it->nal_sz = 0;

	it->profile = NULL;
}
