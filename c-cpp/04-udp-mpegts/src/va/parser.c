#include "va.h"


int va_parser_new(VAParser **out) {
	VAParser *it = NULL;

	it = calloc(1, sizeof(VAParser));
	*out = it;

	return 0;
}

int va_parser_open(VAParser *it, VAParserOpenArgs *args) {
	int ret = 0;
	char ebuf[255];

	if (!args) { ret = 1; goto cleanup; }
	url_parse(&it->i, args->i_url_raw);

	it->udp = udp_new();             /* i */
	fifo_new(&it->fifo);             /* o */
	fifo_init(it->fifo, 100*7*188);
	it->multi = io_multi_writer_new(NULL, 0);  /* o */

	io_reader_new(&it->reader_udp);
	io_reader_new(&it->reader_fifo);
	io_writer_new(&it->writer_fifo);
	io_writer_new(&it->writer_multi);

	io_reader_init(it->reader_udp, it->udp, udp_read);
	io_reader_init(it->reader_fifo, it->fifo, fifo_read);
	io_reader_init(it->writer_fifo, it->fifo, fifo_write);
	io_reader_init(it->writer_multi, it->multi, io_multi_writer_write);

	if ((!it->udp) || (!it->fifo) || (!it->multi) ||
	    (!it->reader_udp) || (!it->reader_fifo) ||
	    (!it->writer_fifo) || (!it->writer_multi)) {
		fprintf(stderr, "[va @ %p] error allocating memory for structures\n", it);
		ret = 1; goto cleanup;
	}

	io_multi_writer_push(it->multi, it->writer_fifo);

	if (udp_open_i(it->udp, url_host(&it->i), it->i.port, NULL,
	               ebuf, sizeof(ebuf))) {
		fprintf(stderr, "[va @ %p] [udp-i @ %p] open error: \"%s\"\n", it, it->udp, ebuf);
		ret = 1; goto cleanup;
	} else
		printf("[va @ %p] [udp-i @ %p] OK {"
			"\"sock\": %d"
			", \"udp-multicast-group\": \"%s\""
			", \"port\": %d"
			", \"if\": \"%s\""
		"}\n", it, it->udp, it->udp->sock, url_host(&it->i), it->i.port, "-");

	it->worker_read.reader = it->reader_udp;
	it->worker_read.writer = it->writer_multi;

	it->worker_parse.fifo = it->fifo;
	it->worker_parse.cb = args->cb;
	it->worker_parse.cb_ctx = args->cb_ctx;

cleanup:
	return ret;
}

int va_parser_go(VAParser *it) {
	int ret = 0;

	if (pthread_create(&it->worker_read.thread, NULL, parser_worker_read_do, (void*)&it->worker_read)) {
		fprintf(stderr, "pthread-create error: \"%s\"\n", strerror(errno));
		ret = 1; goto cleanup;
	} else
		printf("[read-worker @ %p] OK \n", &it->worker_read);

	if (pthread_create(&it->worker_parse.thread, NULL, parser_worker_parse_do, (void*)&it->worker_parse)) {
		fprintf(stderr, "pthread-create error: \"%s\"\n", strerror(errno));
		ret = 1; goto cleanup;
	} else
		printf("[parse-worker @ %p] OK \n", &it->worker_parse);

cleanup:
	return 0;
}

int va_parser_close(VAParser *it) {
	int ret = 0;

	if (!it) return ret;

	udp_del(it->udp);
	fifo_del(it->fifo);
	/* it->multi; */
	io_reader_del(&it->reader_udp);
	io_reader_del(&it->reader_fifo);
	io_writer_del(it->writer_fifo);
	io_writer_del(it->writer_multi);

	it->udp = NULL;
	it->fifo = NULL;
	it->reader_udp = NULL;
	it->reader_fifo = NULL;
	it->writer_fifo = NULL;
	it->writer_multi = NULL;

	return ret;
}

int va_parser_del1(VAParser *it) {
	int ret = 0;

	if (!it) return ret;
	if (!(ret=va_parser_close(it))) free(it);

	return ret;
}

int va_parser_del2(VAParser **it) {
	int ret = 0;

	if (!it) return ret;
	if (!(ret=va_parser_del1(*it))) *it = NULL;

	return ret;
}
