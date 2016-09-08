#ifndef __URL__
#define __URL__


#include <stdio.h>  // print
#include <stdlib.h> // malloc, NULL
#include <string.h> // strstr
#include <stdint.h> // uint8_t


/* RFC 3986 */
struct _URL {
	char   *scheme;
	char   *host;
	uint8_t port;
};

typedef struct _URL URL;

URL *url_parse(char *raw);


#endif // __URL__
