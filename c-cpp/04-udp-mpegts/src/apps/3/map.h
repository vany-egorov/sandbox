#ifndef __APPS_3_MAP__
#define __APPS_3_MAP__


#include <stdio.h>   /* snprintf */
#include <stdint.h>  /* uint32_t */
#include <stddef.h>  /* size_t */
#include <stdlib.h>  /* strtol */
#include <string.h>  /* strcasecmp, strcmp, strncpy */


typedef enum   map_kind_enum MapKind;
typedef struct map_s         Map;


enum map_kind_enum {
	MAP_UNK = 0,  /* unknown */

	MAP_ALL = 1,  /* all */

	MAP_V = 2,  /* video */
	MAP_A = 3,  /* audio */
	MAP_S = 4,  /* subtitle */
	MAP_T = 5,  /* teletext */

	MAP_ID = 6,  /* ID/PID */
};


MapKind map_kind_parse(char *v);
char *map_kind_str(MapKind it);


struct map_s {
	MapKind kind;
	uint32_t id;  /* ID/PID */
};


int map_parse(Map *it, char *v);
int map_str(Map *it, char *buf, size_t bufsz);


#endif  /* __APPS_3_MAP__ */
