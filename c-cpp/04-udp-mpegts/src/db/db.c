#include "db.h"


int db_new(DB **out) {
	DB *it = NULL;

	it = calloc(1, sizeof(DB));
	*out = it;

	slice_init(&it->timestamps, sizeof(DBTimestamp));
	slice_init(&it->atoms, sizeof(DBAtom));

	return 0;
}

static inline int db_store(DB *it, Slice *slice, void *item, size_t itemsz, DBAtomKind kind, uint64_t offset) {
	int ret = 0;

	if (!it->start_at) time(&it->start_at);

	slice_append(slice, item);

	DBAtom atom = {
		.kind = kind,
		.offset = offset,
		.data = slice_tail(slice),
	};

	slice_append(&it->atoms, &atom);

	return ret;
}


int db_store_mpegts_header(DB *it, MPEGTSHeader *item, uint64_t offset) {
	return db_store(it, &it->mpegts_headers, item, sizeof(MPEGTSHeader), DB_MPEGTS_HEADER, offset); }

int db_store_mpegts_adaptation(DB *it, MPEGTSAdaptation *item, uint64_t offset) {
	return db_store(it, &it->mpegts_adaptations, item, sizeof(MPEGTSAdaptation), DB_MPEGTS_ADAPTION, offset); }

int db_store_mpegts_pes(DB *it, MPEGTSPES *item, uint64_t offset) {
	return db_store(it, &it->mpegts_pess, item, sizeof(MPEGTSPES), DB_MPEGTS_PES, offset); }

int db_store_mpegts_psi_pat(DB *it, MPEGTSPSIPAT *item, uint64_t offset) {
	return db_store(it, &it->mpegts_psi_pats, item, sizeof(MPEGTSPSIPAT), DB_MPEGTS_PSI_PAT, offset); }

int db_store_mpegts_psi_pmt(DB *it, MPEGTSPSIPMT *item, uint64_t offset) {
	return db_store(it, &it->mpegts_psi_pmts, item, sizeof(MPEGTSPSIPMT), DB_MPEGTS_PSI_PMT, offset); }

int db_store_mpegts_psi_sdt(DB *it, MPEGTSPSISDT *item, uint64_t offset) {
	return db_store(it, &it->mpegts_psi_sdts, item, sizeof(MPEGTSPSIPMT), DB_MPEGTS_PSI_SDT, offset); }


int db_store_h264(DB *it, H264NAL *nal, uint64_t offset) {
	int ret = 0;

	switch (nal->type) {
		case H264_NAL_TYPE_SPS: {
			ret = db_store_h264_sps(it, &nal->u.sps, offset);
			break;
		}
		case H264_NAL_TYPE_PPS: {
			ret = db_store_h264_pps(it, &nal->u.pps, offset);
			break;
		}
		case H264_NAL_TYPE_AUD: {
			ret = db_store_h264_aud(it, &nal->u.aud, offset);
			break;
		}
		case H264_NAL_TYPE_SEI: {
			ret = db_store_h264_sei(it, &nal->u.sei, offset);
			break;
		}
		case H264_NAL_TYPE_SLICE:
		case H264_NAL_TYPE_IDR:
			ret = db_store_h264_slice_idr(it, &nal->u.slice_idr, offset);
			break;
	}

cleanup:
	return ret;
}

int db_store_h264_sps(DB *it, H264NALSPS *item, uint64_t offset) {
	return db_store(it, &it->h264_spss, item, sizeof(H264NALSPS), DB_H264_SPS, offset); }

int db_store_h264_pps(DB *it, H264NALPPS *item, uint64_t offset) {
	return db_store(it, &it->h264_ppss, item, sizeof(H264NALPPS), DB_H264_PPS, offset); }

int db_store_h264_aud(DB *it, H264NALAUD *item, uint64_t offset) {
	return db_store(it, &it->h264_auds, item, sizeof(H264NALAUD), DB_H264_AUD, offset); }

int db_store_h264_sei(DB *it, H264NALSEI *item, uint64_t offset) {
	return db_store(it, &it->h264_seis, item, sizeof(H264NALSEI), DB_H264_SEI, offset); }

int db_store_h264_slice_idr(DB *it, H264NALSliceIDR *item, uint64_t offset) {
	return db_store(it, &it->h264_slice_idrs, item, sizeof(H264NALSliceIDR), DB_H264_SLICE_IDR, offset); }


void db_del(DB **it) {
	if (!it) return;
	if (!*it) return;

	free(*it);
	*it = NULL;

	return;
}
