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
	char buf[255];

	if (it->i) {
		memset(&buf, 0, sizeof(buf));
		url_sprint(it->i, buf, sizeof(buf));
		printf("src/input: \"%s\"\n", buf);
	} else
		printf("src/input: \"-\"\n");

	if (it->o) {
		memset(&buf, 0, sizeof(buf));
		url_sprint(it->o, buf, sizeof(buf));
		printf("dst/input: \"%s\"\n", buf);
	} else
		printf("dst/input: \"-\"\n");
}

int config_validate(Config *it) {
	int ret = 0;

	if (!it->i) {
		COLORSTDERR("--src / --input / -s / -i : provide source/input;");
		ret = 1;
	} else if (it->i->scheme != URL_SCHEME_UDP) {
		COLORSTDERR("--src / --input / -s / -i : only UDP source supported;");
		ret = 1;
	}

	return ret;
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

		// $1
		if ((i==1) &&
		    (key[0] != '-')) {
			if (!it->i) it->i = calloc(1, sizeof(URL));
			url_parse(it->i, key);
			continue;
		}

		// $2
		if ((i==2) &&
				(key[0] != '-') &&     // positional argument
		    (argv[1][0] != '-') && // no flag before
		    (it->i)) {
			if (!it->o) it->o = calloc(1, sizeof(URL));
			url_parse(it->o, key);
			continue;
		}

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

		// -s | -i | --src
		if (CFGISOPTION(key, "src") || CFGISOPTION(key, "s") || CFGISOPTION(key, "i")) {
			if (!it->i) it->i = calloc(1, sizeof(URL));
			url_parse(it->i, argv[i+1]);
		}

		// -d | -o | --dst
		if (CFGISOPTION(key, "dst") || CFGISOPTION(key, "d") || CFGISOPTION(key, "o") || CFGISOPTION(key, "out")) {
			if (!it->o) it->o = calloc(1, sizeof(URL));
			url_parse(it->o, argv[i+1]);
		}
	}
}
