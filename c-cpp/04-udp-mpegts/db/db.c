#include "db.h"


int db_new(DB **out) {
	DB *it = NULL;

	it = calloc(1, sizeof(DB));
	*out = it;

	slice_new(&it->timestamps, sizeof(DBTimestamp));
	slice_new(&it->atoms, sizeof(DBAtom));

	it->mpegts_headers = NULL;
	it->mpegts_adaptions = NULL;
	it->mpegts_pess = NULL;
	it->mpegts_psi_pats = NULL;
	it->mpegts_psi_pmts = NULL;
	it->mpegts_psi_sdts = NULL;

	it->h264_auds = NULL;
	it->h264_seis = NULL;
	it->h264_spss = NULL;
	it->h264_ppss = NULL;
	it->h264_slice_idrs = NULL;

	return 0;
}

int db_store_mpegts_header(DB *it, MPEGTSHeader *item, uint64_t offset) {
	if (!it->start_at) time(&it->start_at);
	if (!it->mpegts_headers) slice_new(&it->mpegts_headers, sizeof(MPEGTSHeader));

	slice_append(it->mpegts_headers, item);

	DBAtom atom = {
		.kind = DB_MPEGTS_HEADER,
		.offset = offset,
		.data = slice_tail(it->mpegts_headers),
	};

	slice_append(it->atoms, &atom);

	return 0;
}

int db_store_mpegts_adaption(DB *it, MPEGTSAdaption *item, uint64_t offset) {
	if (!it->start_at) time(&it->start_at);
	if (!it->mpegts_adaptions) slice_new(&it->mpegts_adaptions, sizeof(MPEGTSAdaption));

	slice_append(it->mpegts_adaptions, item);

	DBAtom atom = {
		.kind = DB_MPEGTS_ADAPTION,
		.offset = offset,
		.data = slice_tail(it->mpegts_adaptions),
	};

	slice_append(it->atoms, &atom);

	return 0;
}

int db_store_mpegts_pes(DB *it, MPEGTSPES *item, uint64_t offset) {
	if (!it->start_at) time(&it->start_at);
	if (!it->mpegts_pess) slice_new(&it->mpegts_pess, sizeof(MPEGTSPES));

	slice_append(it->mpegts_pess, item);

	DBAtom atom = {
		.kind = DB_MPEGTS_PES,
		.offset = offset,
		.data = slice_tail(it->mpegts_pess),
	};

	slice_append(it->atoms, &atom);

	return 0;
}

int db_store_h264_sps(DB *it) {
	printf("%lu\n", time(NULL));
}

void db_del(DB **it) {
	if (!it) return;
	if (!*it) return;

	free(*it);
	*it = NULL;

	return;
}
