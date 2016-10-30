#include "io.h"


/* reader */
IOReader *io_reader_new(void *w, io_reader_read_func read) {
	IOReader *it = calloc(1, sizeof(IOReader));
	if (!it) return it;

	it->w = w;
	it->read = read;

	return it;
}

int io_reader_read(IOReader *it, uint8_t *buf, size_t bufsz, size_t *n) {
	return it->read((void*)it->w, buf, bufsz, n);
}

void io_reader_del(IOReader *it) {
	if (!it) return;
	free(it);
}


/* multi-reader */


/* writer */
IOWriter *io_writer_new(void *w, io_writer_write_func write) {
	IOWriter *it = calloc(1, sizeof(IOWriter));
	if (!it) return it;

	it->w = w;
	it->write = write;
	return it;
}

int io_writer_write(IOWriter *it, uint8_t *buf, size_t bufsz, size_t *n) {
	return it->write((void*)it->w, buf, bufsz, n);
}

void io_writer_del(IOWriter *it) {
	if (!it) return;
	free(it);
}


/* multi-writer */
IOMultiWriter *io_multi_writer_new(IOWriter **writers, size_t len) {
	IOMultiWriter *it = calloc(1, sizeof(IOMultiWriter));
	if (!it) return it;

	if (writers) it->writers = writers;
	it->len = len;

	return it;
}

int io_multi_writer_push(IOMultiWriter *it, IOWriter *w) {
	int ret = 0;

	it->writers = (IOWriter**)realloc(it->writers, (it->len + 1)*sizeof(IOWriter*));
	if (!it->writers) { ret = 1; goto cleanup; }
	it->writers[it->len] = w;
	it->len++;

cleanup:
	return ret;
}

// impl Writer for MultiWriter
int io_multi_writer_write(void *ctx, uint8_t *buf, size_t bufsz, size_t *written) {
	int ret = 0,
	    i = 0;
	IOMultiWriter *it = NULL;
	IOWriter *writer = NULL;

	it = (IOMultiWriter*)ctx;

	for (i = 0; i < it->len; i++) {
		writer = it->writers[i];
		if ((ret=io_writer_write(writer, buf, bufsz, written)))
			return ret;
	}

	return ret;
}


/* io */
int io_copy(IOReader* src, IOWriter* dst, uint8_t *buf, size_t bufsz, size_t *copied) {
	int ret = 0;
	size_t readen = 0;

	if ((ret=io_reader_read(src, buf, bufsz, &readen))) {
		return ret;
	}

	if ((ret=io_writer_write(dst, buf, readen, copied))) {
		return ret;
	}

	return ret;
}
