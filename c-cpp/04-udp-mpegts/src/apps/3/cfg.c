#include "cfg.h"


/* memory allocation */
int cfg_new(Cfg **out) {
	int ret = 0;
	Cfg *it = NULL;

	it = calloc(1, sizeof(Cfg));
	if (!it) {
		return 1;
	}
	*out = it;

	return ret;
}

/* initialize => set initial state */
int cfg_init(Cfg *it) {
	int ret = 0;

	slice_init(&it->i, sizeof(CfgI));

	return ret;
}

int cfg_validate(Cfg *it) {
	int ok = 1;

	if (!it->i.len) {
		fprintf(stderr, "no inputs provided; -i --input: must be specified;\n");
		ok = 0;
	}

	return ok;
}

int cfg_print(Cfg *it) {
	CfgI *cfg_i = NULL;
	CfgMap *cfg_map = NULL;
	CfgO *cfg_o = NULL;
	char buf[255] = { 0 };

	printf("inputs:\n");
	{int i = 0; for (i = 0; i < (int)it->i.len; i++) {
		cfg_i = slice_get(&it->i, (size_t)i);

		memset(buf, 0, sizeof(buf));
		url_sprint(&cfg_i->url, buf, sizeof(buf));

		printf("  - id: %" PRIu64 "\n", cfg_i->id);
		if (cfg_i->name && cfg_i->name[0] != '\0')
			printf("    name: \"%s\"\n", cfg_i->name);
		printf("    url: \"%s\"\n", buf);

		if (cfg_i->maps.len) {
			printf("    maps:\n");
			{int j = 0; for (j = 0; j < (int)cfg_i->maps.len; j++) {
				cfg_map = slice_get(&cfg_i->maps, (size_t)j);

				memset(buf, 0, sizeof(buf));
				map_str(&cfg_map->map, buf, sizeof(buf));

				printf("      \"%s\":\n", buf);
				{int k = 0; for (k = 0; k < (int)cfg_map->o.len; k++) {
					cfg_o = slice_get(&cfg_map->o, (size_t)k);

					memset(buf, 0, sizeof(buf));
					url_sprint(&cfg_o->url, buf, sizeof(buf));

					printf("        -o: {url: \"%s\"}\n", buf);
				}}
			}}
		}

	}}

	return 0;
}

/* finalize */
int cfg_fin(Cfg *it) {
	int ret = 0;

	if (!it) return ret;

	if (!it->i.len)
		slice_fin(&it->i);

	return ret;
}

/* destructor */
int cfg_del(Cfg **out) {
	int ret = 0;
	Cfg *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	cfg_fin(it);

	free(it);
	*out = NULL;

	return ret;
}


int cfg_i_init(CfgI *it) {
	slice_init(&it->maps, sizeof(CfgMap));
	return 0;
}

int cfg_i_fin(CfgI *it) {
	slice_fin(&it->maps);
	return 0;
}


int cfg_map_init(CfgMap *it) {
	slice_init(&it->o, sizeof(CfgO));
	return 0;
}

int cfg_map_fin(CfgMap *it) {
	slice_fin(&it->o);
	return 0;
}
