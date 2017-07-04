#include "./input.h"  /* InputVt */
#include "./input-file.h"


int input_file_new(InputFile **out) {
	int ret = 0;
	InputFile *it = NULL;

	it = calloc(1, sizeof(InputFile));
	if (!it)
		return 1;
	*out = it;

	return ret;
}

static int input_file_open(void *ctx, URL *u) {
	char ebuf[255] = { 0 };
	char us[255] = { 0 }; /* url string */
	InputFile *it = NULL;

	it = (InputFile*)ctx;
	url_sprint(u, us, sizeof(us));

	if (file_open(&it->i, url_path(u), "rb", ebuf, sizeof(ebuf))) {
		// TODO: move to logger
		fprintf(stderr, "[input-file @ %p] ERROR open %s, reason: %s\n",
			it, us, ebuf);
 	} else {
 		// TODO: move to logger
 		printf("[input-file @ %p] OK open %s\n", it, us);
 	}
}

int input_file_read(void *ctx, uint8_t *buf, size_t bufsz, size_t *n) {
}

int input_file_close(void *ctx) {
}


InputVt input_file_vt = {
	.open = input_file_open,
	.read = input_file_read,
	.close = input_file_close,
};
