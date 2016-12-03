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

static inline int db_store(DB *it, Slice **slice, void *item, size_t itemsz, DBAtomKind kind, uint64_t offset) {
	if (!it->start_at) time(&it->start_at);
	if (!*slice) slice_new(slice, itemsz);

	slice_append(*slice, item);

	DBAtom atom = {
		.kind = kind,
		.offset = offset,
		.data = slice_tail(*slice),
	};

	slice_append(it->atoms, &atom);

	return 0;
}

int db_store_mpegts_header(DB *it, MPEGTSHeader *item, uint64_t offset) {
	return db_store(it, &it->mpegts_headers, item, sizeof(MPEGTSHeader), DB_MPEGTS_HEADER, offset); }

int db_store_mpegts_adaption(DB *it, MPEGTSAdaption *item, uint64_t offset) {
	return db_store(it, &it->mpegts_adaptions, item, sizeof(MPEGTSAdaption), DB_MPEGTS_ADAPTION, offset); }

int db_store_mpegts_pes(DB *it, MPEGTSPES *item, uint64_t offset) {
	return db_store(it, &it->mpegts_pess, item, sizeof(MPEGTSPES), DB_MPEGTS_PES, offset); }

int db_store_mpegts_psi_pat(DB *it, MPEGTSPSIPAT *item, uint64_t offset) {
	return db_store(it, &it->mpegts_psi_pats, item, sizeof(MPEGTSPSIPAT), DB_MPEGTS_PSI_PAT, offset); }

int db_store_mpegts_psi_pmt(DB *it, MPEGTSPSIPMT *item, uint64_t offset) {
	return db_store(it, &it->mpegts_psi_pmts, item, sizeof(MPEGTSPSIPMT), DB_MPEGTS_PSI_PMT, offset); }

int db_store_mpegts_psi_sdt(DB *it, MPEGTSPSISDT *item, uint64_t offset) {
	return db_store(it, &it->mpegts_psi_sdts, item, sizeof(MPEGTSPSIPMT), DB_MPEGTS_PSI_SDT, offset); }

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
