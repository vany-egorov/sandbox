#include "db.h"


int db_new(DB **out) {
	DB *it = NULL;

	it = calloc(1, sizeof(Slice));
	*out = it;

	it->atoms = NULL;

	it->mpegts_tss = NULL;
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

void db_del(DB **it) {
	if (!it) return;
	if (!*it) return;

	free(*it);
	*it = NULL;

	return;
}
