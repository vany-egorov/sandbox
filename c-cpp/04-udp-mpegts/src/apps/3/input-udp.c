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

static void* fifo_reader(void *args) {
	InputUDP *it = NULL;
	IOReader r = { 0 };
	IOWriter w = { 0 };
	uint8_t buf[MPEGTS_PACKET_COUNT*MPEGTS_PACKET_SIZE] = { 0 };  /* TODO: move buffer size to config */
	size_t copied = 0;

	it = (InputUDP*)args;

	io_reader_init(&r, &it->i, udp_read);
	io_writer_init(&w, &it->fifo, fifo_write);

	for (;;)
		io_copy(&r, &w, buf, sizeof(buf), &copied);
}

static int opn(void *ctx, URL *u) {
	char ebuf[255] = { 0 };
	char us[255] = { 0 }; /* url string */
	InputUDP *it = NULL;

	it = (InputUDP*)ctx;

	url_sprint(u, us, sizeof(us));

	if (udp_open_i(&it->i, url_host(u), u->port,
	               NULL, ebuf, sizeof(ebuf))) {
		fprintf(stderr, "[input-udp @ %p] ERROR open %s, reason: %s\n", (void*)it, us, ebuf);  /* TODO: move to logger, error code */
	} else {
		printf("[input-udp @ %p] OK open %s\n", it, us);

		if (fifo_init(&it->fifo, 100*7*188)) {
			fprintf(stderr, "[input-udp @ %p] ERROR init fifo\n", (void*)it);
		} else {
			printf("[input-udp @ %p] OK init fifo\n", (void*)it);  /* TODO: move to logger */

			if (pthread_create(&it->_thrd, NULL, fifo_reader, (void*)it)) {
				fprintf(stderr, "[input-udp @ %p] ERROR spawn new thread\n", (void*)it);  /* TODO: move to logger */
			} else {
				printf("[input-udp @ %p] OK spawn new reader thread\n", (void*)it);  /* TODO: move to logger */
			}
		}
	}
}

static int rd(void *ctx, uint8_t *buf, size_t bufsz, size_t *n) {
}

static int cls(void *ctx) {

}

InputVt input_udp_vt = {
	.open = opn,
	.read = rd,
	.close = cls,
};
