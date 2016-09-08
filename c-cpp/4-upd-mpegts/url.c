#include "url.h"

URL *url_parse(char *raw) {
	char *has_scheme = strstr(raw, "://");
	printf("%s\n", has_scheme);
	return NULL;
}
