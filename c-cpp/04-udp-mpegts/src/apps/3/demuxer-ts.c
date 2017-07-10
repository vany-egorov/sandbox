#include "demuxer-ts.h"


static Logger *lgr = &logger_std;


int demuxer_ts_new(DemuxerTS **out) {
	int ret = 0;
	DemuxerTS *it = NULL;

	it = calloc(1, sizeof(DemuxerTS));
	if (!it)
		return 1;
	*out = it;

	return ret;
}

int demuxer_ts_init(DemuxerTS *it) {
}


static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	int ret = 0;
	uint8_t *cursor = NULL;
	MPEGTSHeader mpegts_header = { 0 };
	MPEGTSAdaption mpegts_adaption = { 0 };
	char sbuf[255] = { 0 };

	cursor = buf;

	if (cursor[0] != MPEGTS_SYNC_BYTE) return 1;

	cursor++;

	mpegts_header_parse(&mpegts_header, cursor); cursor += 3;

	if (mpegts_header.adaption_field_control) {
		mpegts_adaption_parse(&mpegts_adaption, cursor);

		if (mpegts_adaption.PCR_flag) {
			mpegts_pcr_sprint_json(&mpegts_adaption.PCR, sbuf, sizeof(sbuf));
			log_trace(lgr, "[demux-ts @ %p] %s\n", ctx, sbuf);
		}
	}

	return ret;
}


DemuxerVT demuxer_ts_vt = {
	.consume_pkt_raw = consume_pkt_raw,
};
