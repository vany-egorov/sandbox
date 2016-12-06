#ifndef __VA_VA__
#define __VA_VA__


#include <stddef.h>  /* NULL */
#include <stdlib.h>  /* calloc, realloc */

#include "../url/url.h"
#include "../io/io.h"
#include "../collections/fifo.h"
#include "../mpegts/mpegts.h"
#include "../h264/h264.h"

#include "atom-kind.h"


typedef struct va_parser_s              VAParser;
typedef struct va_parser_open_args_s    VAParserOpenArgs;
typedef struct va_parser_read_worker_s  VAParserReadWorker;
typedef struct va_parser_parse_worker_s VAParserParseWorker;
typedef int (*va_parser_parse_cb_func) (void *ctx);

/* parser.c */
struct va_parser_read_worker_s {
	IOReader *reader;
	IOWriter *writer;
};

struct va_parser_parse_worker_s {
	FIFO *fifo;

	MPEGTS mpegts;
	H264   h264;

	uint64_t offset;
	uint16_t video_PID_H264;
};

struct va_parser_s {
	URL i; /* i / input */

	VAParserReadWorker  read_worker;
	VAParserParseWorker parse_worker;
};

struct va_parser_open_args_s {
	const char             *i_url_raw;
	va_parser_parse_cb_func cb;
};

int va_parser_new(VAParser **out);
int va_parser_open(VAParser *it, VAParserOpenArgs *args);
int va_parser_go(VAParser *it);
int va_parser_del(VAParser **it);


#endif /* __VA_VA__ */
