#ifndef __VA_VA__
#define __VA_VA__


#include <stddef.h>  /* NULL */
#include <stdlib.h>  /* calloc, realloc */

#include "atom-kind.h"


typedef struct va_parser_s           VAParser;
typedef struct va_parser_open_args_s VAParserOpenArgs;
typedef int (*va_parser_parse_cb_func) (void *ctx);

/* parser.c */
struct va_parser_s {
	int i;
};

struct va_parser_open_args_s {
	const char             *url_raw;
	va_parser_parse_cb_func cb;
};

int va_parser_new(VAParser **out);
int va_parser_open(VAParser *it, VAParserOpenArgs *args);
int va_parser_go(VAParser *it);
int va_parser_del(VAParser **it);


#endif /* __VA_VA__ */
