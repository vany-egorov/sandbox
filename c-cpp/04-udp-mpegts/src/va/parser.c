#include "va.h"


int va_parser_new(VAParser **out) {
	VAParser *it = NULL;

	it = calloc(1, sizeof(VAParser));
	*out = it;

	return 0;
}

int va_parser_open(VAParser *it, VAParserOpenArgs *args) {
	return 0;
}

int va_parser_go(VAParser *it) {
	return 0;
}

int va_parser_del(VAParser **it) {
	int ret = 0;

	if (!it) return ret;
	if (!*it) return ret;

	free(*it);
	*it = NULL;

	return ret;
}
