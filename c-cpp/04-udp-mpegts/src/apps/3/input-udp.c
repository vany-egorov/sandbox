#include "./input.h"  /* InputVt */
#include "./input-udp.h"


int input_udp_new(InputUDP **out) {
	int ret = 0;
	InputUDP *it = NULL;

	it = calloc(1, sizeof(InputUDP));
	if (!it)
		return 1;
	*out = it;

	return ret;
}

static int input_udp_open(void *ctx, URL *u) {
	char ebuf[255] = { 0 };
	char us[255] = { 0 }; /* url string */
	InputUDP *it = NULL;

	it = (InputUDP*)ctx;

	url_sprint(u, us, sizeof(us));

	if (udp_open_i(&it->i, url_host(u), u->port,
	               NULL, ebuf, sizeof(ebuf))) {
		fprintf(stderr, "[input-udp @ %p] ERROR open %s, reason: %s\n", it, us, ebuf);
	} else
		printf("[input-udp @ %p] OK open %s\n", it, us);
}

static int input_udp_read(void *ctx, uint8_t *buf, size_t bufsz, size_t *n) {
}

static int input_udp_close(void *ctx) {

}

InputVt input_udp_vt = {
	.open = input_udp_open,
	.read = input_udp_read,
	.close = input_udp_close,
};
