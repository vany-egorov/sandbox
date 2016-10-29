#ifndef __CONFIG__
#define __CONFIG__


#include <stdio.h>  // printf
#include <stdlib.h> // malloc, NULL
#include <stdint.h> // uint8_t

#include "url.h"
#include "color.h"


struct _Config {
	uint8_t version;
	uint8_t help;

	URL *i;
	URL *o;
};

typedef struct _Config Config;

Config *config_new();
int config_parse(Config *it, int argc, char **argv);
void config_print(Config *it);
void config_help(void);

#endif // __CONFIG__
