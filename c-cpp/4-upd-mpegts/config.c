#include "config.h"


#define CFGISOPTION(X, Y)  \
	(!strcmp(X, "-"Y))    || \
	(!strcmp(X, "--"Y))   || \
	(!strcmp(X, "-"Y":")) || \
	(!strcmp(X, "--"Y":"))   \


Config *config_new(void) {
	Config *it = malloc(sizeof(Config));
	if (it == NULL)
		return NULL;

	it->version = 0;
	it->help = 0;

	it->i = NULL;
	it->o = NULL;

	return it;
}

void config_del(Config *it) {
	if (it !=NULL)
		free(it);
}


void config_print(Config *it) {
	if (it->i) printf("input: %s\n", it->i);
	if (it->o) printf("output: %s\n", it->o);
}

void config_help(void) {
	printf("Usage:\n");
	printf("\t%s -v, --version %s         | print version and exit\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
	printf("\t%s -h, --help %s            | print help and exit\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
	printf("\t%s -i, --input %s           | <str>  | <mandatory> | Where to read from. File or multicast address:\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
	printf("\t                                 . \t udp://239.1.1.1:5510 - UDP multicast\n");
	printf("\t                                 . \t file:///tmp/in.ts    - file\n");
	printf("\t%s -o, --output %s          | <str>  | Where to write to:\n", COLOR_BRIGHT_GREEN, COLOR_RESET);
	printf("\t                                 . \t file:///tmp/out.ts   - file\n");
}

/** command-line SAX parser **/
int config_parse(Config *it, int argc, char **argv) {
	int i = 0;
	char *key = NULL;

	for (i = 0; i < argc; i++) {
		key = argv[i];

		// -v | --version
		if (CFGISOPTION(key, "version") || CFGISOPTION(key, "v")) {
			it->version = 1;
			return 0;
		}

		// -h | --help
		if (CFGISOPTION(key, "help") || CFGISOPTION(key, "h")) {
			it->help = 1;
			return 0;
		}

		// -i | --input
		if (CFGISOPTION(key, "i") || CFGISOPTION(key, "input") || CFGISOPTION(key, "in")) {
			it->i = argv[i+1];
			continue;
		}

		// -o | --output
		if (CFGISOPTION(key, "o") || CFGISOPTION(key, "output") || CFGISOPTION(key, "out")) {
			it->o = argv[i+1];
			continue;
		}
	}
}
