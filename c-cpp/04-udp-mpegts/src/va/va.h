#ifndef __VA_VA__
#define __VA_VA__


#include <stddef.h>  /* NULL */
#include <stdlib.h>  /* calloc, realloc */
#include <stdarg.h>  /* __gnuc_va_list, __builtin_va_list */

#include "../url/url.h"
#include "../io/io.h"
#include "../io/udp.h"
#include "../collections/fifo.h"
#include "../mpegts/mpegts.h"
#include "../h264/h264.h"

#include "./atom-kind.h"


typedef struct va_atom_wrapper_s        VAAtomWrapper;
typedef struct va_parser_s              VAParser;
typedef struct va_parser_open_args_s    VAParserOpenArgs;
typedef struct va_parser_worker_read_s  VAParserWorkerRead;
typedef struct va_parser_worker_parse_s VAParserWorkerParse;
typedef int (*va_parser_parse_cb_func) (void *ctx, VAAtomWrapper *atom);


struct va_atom_wrapper_s {
	uint64_t id; /* index */
	uint64_t offset;

	VAAtomKind kind;
	void *atom;
};


/* parser-worker-read.c */
struct va_parser_worker_read_s {
	IOReader *reader;
	IOWriter *writer;

	pthread_t thread;
};

void* parser_worker_read_do(void *args);


/* parser-worker-parse.c */
struct va_parser_worker_parse_s {
	FIFO *fifo;

	MPEGTS mpegts;
	H264   h264;

	H264AnnexBParseResult *h264_annex_b_parse_result; /* last/prev parse result */

	void *cb_ctx;
	va_parser_parse_cb_func cb;

	uint64_t id; /* index */
	uint64_t offset;
	uint16_t video_PID_H264;

	pthread_t thread;
};

void* parser_worker_parse_do(void *args);

/* parser.c */
struct va_parser_s {
	URL i; /* i / input */

	UDP *udp;              /* i */
	FIFO *fifo;            /* o */
	IOMultiWriter *multi;
	IOReader *reader_udp,
	         *reader_fifo;
	IOWriter *writer_fifo,
	         *writer_multi;


	VAParserWorkerRead  worker_read;
	VAParserWorkerParse worker_parse;
};

struct va_parser_open_args_s {
	const char             *i_url_raw;

	void *cb_ctx;
	va_parser_parse_cb_func cb;
};

int va_parser_new(VAParser **out);
int va_parser_open(VAParser *it, VAParserOpenArgs *args);
int va_parser_go(VAParser *it);
int va_parser_close(VAParser *it);
int va_parser_del1(VAParser *it);
int va_parser_del2(VAParser **it);


#endif /* __VA_VA__ */
