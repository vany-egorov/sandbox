#include "./input.h"
#include "./input-udp.h"   /* InputUDP */
#include "./input-file.h"  /* InputFile */


int input_new(Input **out) {
	int ret = 0;
	Input *it = NULL;

	it = calloc(1, sizeof(Input));
	if (!it)
		return 1;
	*out = it;

	return ret;
}

int input_open(Input *it, URL *u) {
	char ebuf[255] = { 0 };
	char us[255] = { 0 };  /* url string */
	url_sprint(u, us, sizeof(us));

	/* guess input format */
	switch (u->scheme) {
		case URL_SCHEME_UDP: {
			InputUDP *i = NULL;
			input_udp_new(&i);
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

	if ((it->vt) && (it->w))
		it->vt->open(it->w, u);
}

int input_read(Input *it) {
	if ((!it->vt) || (!it->w)) return 0;

	size_t n = 0;
	it->vt->read(it->w, NULL, 0, &n);
}

int input_close(Input *it) {

}
