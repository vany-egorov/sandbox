#include "map.h"

MapKind map_kind_parse(char *v) {
	if (!strcasecmp(v, "all")) {
		return MAP_ALL;

	} else if (!strcasecmp(v, "a") || !strcasecmp(v, "audio")) {
		return MAP_A;
	} else if (!strcasecmp(v, "v") || !strcasecmp(v, "video")) {
		return MAP_V;
	} else if (!strcasecmp(v, "s") || !strcasecmp(v, "subtitle")) {
		return MAP_S;
	} else if (!strcasecmp(v, "t") || !strcasecmp(v, "teletext")) {
		return MAP_T;
	}

	return MAP_UNK;
}

char *map_kind_str(MapKind it) {
	switch (it) {
	case MAP_ALL: return "all";

	case MAP_A: return "audio";
	case MAP_V: return "video";
	case MAP_S: return "subtitle";
	case MAP_T: return "teletext";

	case MAP_ID: return "id/pid";

	case MAP_UNK: return "unk";

	default: return "unk";
	}

	return "unk";
}

int map_parse(Map *it, char *v) {
	uint32_t id = 0;
	MapKind mk = map_kind_parse(v);

	if (mk != MAP_UNK) {
		it->kind = mk;
		it->id = 0;
		goto cleanup;
	}

	if (!strcmp(v, "0")) {
		it->kind = MAP_ID;
		it->id = 0;
	} else {
		id = (uint32_t)strtol(v, NULL, 0);

		if (!id) {
			it->kind = MAP_UNK;
			goto cleanup;
		}

		it->kind = MAP_ID;
		it->id = id;
	}

cleanup:
	return 0;
}

int map_str(Map *it, char *buf, size_t bufsz) {
	int ret = 0;

	if (it->kind == MAP_ID) {
		snprintf(buf, bufsz, "%s:%d|0x%X",
			map_kind_str(it->kind),
			it->id, it->id);
		return ret;
	}

	strncpy(buf, map_kind_str(it->kind), bufsz);
	return ret;
}

int map_match(Map *it, CodecKind codec, uint32_t id) {
	if ((it->kind == MAP_A) && (codec & CODEC_KIND_AUDIO)) return 1;
	if ((it->kind == MAP_V) && (codec & CODEC_KIND_VIDEO)) return 1;
	if ((it->kind == MAP_T) && (codec & CODEC_KIND_TELETEXT)) return 1;
	if ((it->kind == MAP_S) && (codec & CODEC_KIND_SUBTITLE)) return 1;

	if ((it->kind == MAP_ID) && (it->id == id)) return 1;

	return 0;
}
