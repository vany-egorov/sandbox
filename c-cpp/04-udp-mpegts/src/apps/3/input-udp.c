#include "./input-udp.h"


static Logger *lgr = &logger_std;


int input_udp_new(InputUDP **out) {
	int ret = 0;
	InputUDP *it = NULL;

	it = calloc(1, sizeof(InputUDP));
	if (!it)
		return 1;
	*out = it;

	return ret;
}

int input_udp_init(InputUDP *it, InputUDPCfg *c) {
	int ret = 0;

	it->c = *c;

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
		log_error(lgr, "[input-udp @ %p] ERROR open %s, reason: %s\n", (void*)it, us, ebuf);  /* TODO: move to logger, error code */
	} else {
		log_info(lgr, "[input-udp @ %p] OK open %s\n", it, us);

		if (fifo_init(&it->fifo, it->c.fifo_cap)) {
			log_error(lgr, "[input-udp @ %p] ERROR init fifo\n", (void*)it);
		} else {
			log_debug(lgr, "[input-udp @ %p] OK init fifo, cap: %zu\n", (void*)it, it->c.fifo_cap);

			if (pthread_create(&it->_thrd, NULL, fifo_reader, (void*)it)) {
				log_error(lgr, "[input-udp @ %p] ERROR spawn new thread\n", (void*)it);  /* TODO: move to logger */
			} else {
				log_debug(lgr, "[input-udp @ %p] OK spawn new reader thread\n", (void*)it);  /* TODO: move to logger */
			}
		}
	}
}

static int rd(void *ctx, void *opaque, input_read_cb_fn cb) {
	InputUDP *it = NULL;
	size_t rln = 0,  /* read length */
	       fifo_ln = 0;  /* fifo length */
	uint8_t pkt[MPEGTS_PACKET_SIZE] = { 0 };

	it = (InputUDP*)ctx;

	fifo_wait_data(&it->fifo);
	fifo_ln = fifo_len(&it->fifo);

	{int i = 0; for (i = 0; i < fifo_ln; i += MPEGTS_PACKET_SIZE) {
		fifo_read(&it->fifo, pkt, sizeof(pkt), &rln);
		cb(opaque, pkt, sizeof(pkt));
	}}
}

static int cls(void *ctx) {

}

InputVT input_udp_vt = {
	.open = opn,
	.read = rd,
	.close = cls,
};
