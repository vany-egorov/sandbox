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
	slice_init(&it->trks, sizeof(Track));
}

static inline int append_track(Stream *it, Track *trk) {
	trk->i = (uint32_t)it->trks.len;
	slice_append(&it->trks, trk);
}

int stream_from_mpegts_psi_pmt(Stream *it, MPEGTSPSIPMT *psi_pmt) {
	MPEGTSPSIPMTProgramElement *pe = NULL;
	Track trk = { 0 };

	{int i = 0; for (i = 0; i < psi_pmt->program_elements.len; i++) {
		pe = &psi_pmt->program_elements.c[i];

		trk.id = (uint32_t)pe->elementary_PID;
		trk.codec_kind = codec_kind_from_mpegts_es_type(pe->stream_type);
		packet_init(&trk.pkt);

		/* if (trk.codec_kind == CODEC_KIND_UNKNOWN) continue; */

		append_track(it, &trk);
	}}
}

int stream_get_track(Stream *it, uint32_t id, Track **out) {
	int ret = 0;
	Track *trk = NULL;

	if (!it->trks.len) {
		ret = 2;  /* !ok; error -> tracks are not initialized */
		goto cleanup;
	}

	{int i = 0; for (i = 0; i < (int)it->trks.len; i++) {
		trk = slice_get(&it->trks, (size_t)i);
		if (trk->id == id) {
			*out = trk;
			goto cleanup;
		}
	}}

	ret = 1;  /* !ok; error -> not found */
	*out = NULL;

cleanup:
	return ret;
}

int stream_fin(Stream *it) {
	int ret = 0;

	if (!it) return ret;

	if (it->trks.len)
		slice_fin(&it->trks);

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
