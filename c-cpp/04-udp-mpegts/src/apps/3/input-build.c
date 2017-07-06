#include "input-build.h"


int input_build(Input *it, URLProtocol protocol, InputCfg *c) {
	switch (protocol) {
		case URL_SCHEME_UDP: {
			InputUDP *i = NULL;
			input_udp_new(&i);
			input_udp_init(i, &c->udp);
			it->w = (void*)i;
			it->vt = &input_udp_vt;

			break;
		}
		case URL_SCHEME_FILE: {
			InputFile *i = NULL;
			input_file_new(&i);
			it->w = (void*)i;
			it->vt = &input_file_vt;

			break;
		}
		default:
			break;
	}
}
