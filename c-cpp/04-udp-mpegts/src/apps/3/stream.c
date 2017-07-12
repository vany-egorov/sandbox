#include "stream.h"


int stream_new(Stream **out) {
	int ret = 0;
	Stream *it = NULL;

	it = calloc(1, sizeof(Stream));
	if (!it) {
		return 1;
	}
	*out = it;

	return ret;
}

int stream_init(Stream *it) {
	it->trks = NULL;
}

static inline int append_track(Stream *it, Track *trk) {
	if (!it->trks) slice_new(&it->trks, sizeof(Track));
	trk->i = (uint32_t)it->trks->len;
	slice_append(it->trks, trk);
}

int stream_from_mpegts_psi_pmt(Stream *it, MPEGTSPSIPMT *psi_pmt) {
	MPEGTSPSIPMTProgramElement *pe = NULL;
	Track trk = { 0 };

	{int i = 0; for (i = 0; i < psi_pmt->program_elements.len; i++) {
		pe = &psi_pmt->program_elements.c[i];

		trk.id = (uint32_t)pe->elementary_PID;
		trk.codec_kind = codec_kind_from_mpegts_es_type(pe->stream_type);

		if (trk.codec_kind == CODEC_KIND_UNKNOWN) continue;

		append_track(it, &trk);
	}}
}

int stream_fin(Stream *it) {
	int ret = 0;

	if (!it) return ret;

	if (it->trks) {
		slice_del(it->trks);
		it->trks = NULL;
	}

	return ret;
}

int stream_del(Stream **out) {
	int ret = 0;
	Stream *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	stream_fin(it);

	free(it);
	*out = NULL;

	return ret;
}
