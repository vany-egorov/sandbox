#include "url.h"

URL *url_parse(char *raw) {
	char *has_scheme = strstr(raw, "://");
	printf("url_parse %s\n", has_scheme);
	return NULL;
}
