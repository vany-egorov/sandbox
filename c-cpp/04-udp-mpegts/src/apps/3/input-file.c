#include "input-file.h"


int input_file_new(InputFile **out) {
	int ret = 0;
	InputFile *it = NULL;

	it = calloc(1, sizeof(InputFile));
	if (!it)
		return 1;
	*out = it;

	return ret;
}

static int opn(void *ctx, URL *u) {
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

static int rd(void *ctx, void *opaque, input_read_cb_fn cb) {
	InputFile *it = NULL;
	size_t rln = 0;  /* read length */
	uint8_t buf[500*MPEGTS_PACKET_SIZE] = { 0 };  /* TODO: move to config and to struct */

	it = (InputFile*)ctx;

	int ret = file_read(&it->i, buf, sizeof(buf), &rln);
	if (ret == FILE_RESULT_OK) {
		{int i = 0; for (i = 0; i < rln; i += MPEGTS_PACKET_SIZE) {
			cb(opaque, &buf[i*MPEGTS_PACKET_SIZE], MPEGTS_PACKET_SIZE);
		}}
	} else if (ret == FILE_RESULT_OK_EOF) {
		file_seek_start(&it->i);
		printf("[input-file @ %p] seek file to start\n", ctx);

		/* <read step> */
		ret = file_read(&it->i, buf, sizeof(buf), &rln);

		{int i = 0; for (i = 0; i < rln; i += MPEGTS_PACKET_SIZE) {
			cb(opaque, &buf[i*MPEGTS_PACKET_SIZE], MPEGTS_PACKET_SIZE);
		}}
		/* </read step> */
	}
}

static int cls(void *ctx) {
}


InputVT input_file_vt = {
	.open = opn,
	.read = rd,
	.close = cls,
};
