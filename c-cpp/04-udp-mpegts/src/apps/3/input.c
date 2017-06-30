#include "input.h"


int input_new(Input **out) {
	int ret = 0;
	Input *it = NULL;

	it = calloc(1, sizeof(Input));
	if (!it)
		return 1;
	*out = it;

	return ret;
}

int input_open(Input *it, URL *url) {
	/* guess input format */
	switch (u->scheme) {
		case URL_SCHEME_UDP:
			printf("UDP\n");
			break;
		case URL_SCHEME_FILE:
			printf("FILE\n");
			break;
		default:
			printf("unknown\n");
	}

	it ((it->vt) && (it->w))
		it->vt->open(it->vt, url);
}

int input_read(Input *it) {

}

int input_close(Input *it) {

}
