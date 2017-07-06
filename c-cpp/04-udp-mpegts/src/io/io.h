#ifndef __IO__
#define __IO__


#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t */
#include <stdlib.h> /* calloc */
#include <string.h> /* strerror */


/* TODO: make buffer as struct with read, write, close */


/* reader */
typedef struct io_reader_s IOReader;
typedef int (*io_reader_read_func) (void *ctx, uint8_t *buf, size_t bufsz, size_t *n);

struct io_reader_s {
	void *w; /* wrapped */

	io_reader_read_func read;
};

int io_reader_new(IOReader **out);
int io_reader_init(IOReader *it, void *w, io_reader_read_func read);
int io_reader_read(IOReader *it, uint8_t *buf, size_t bufsz, size_t *n);
int io_reader_del(IOReader **out);


/* multi-reader */
typedef struct io_multi_reader_s IOMultiReader;

struct io_multi_reader_s {
	IOReader *readers;
	size_t    len;
};


/* writer */
typedef struct io_writer_s IOWriter;
typedef int (*io_writer_write_func) (void *ctx, uint8_t *buf, size_t bufsz, size_t *n);

struct io_writer_s {
	void *w; /* wrapped */

	io_writer_write_func write;
};

int io_writer_new(IOWriter **out);
int io_writer_init(IOWriter *it, void *w, io_writer_write_func write);
int io_writer_write(IOWriter *it, uint8_t *buf, size_t bufsz, size_t *n);
int io_writer_del(IOWriter **out);


/* multi-writer */
typedef struct io_multi_writer_s IOMultiWriter;

struct io_multi_writer_s {
	IOWriter **writers;
	size_t     len;
};

IOMultiWriter *io_multi_writer_new(IOWriter **writers, size_t len);
int io_multi_writer_push(IOMultiWriter *it, IOWriter *w);
/* impl Writer for MultiWriter */
int io_multi_writer_write(void *ctx, uint8_t *buf, size_t bufsz, size_t *written);


/* io */
int io_copy(IOReader* src, IOWriter* dst, uint8_t *buf, size_t bufsz, size_t *copied);


#endif // __IO__
