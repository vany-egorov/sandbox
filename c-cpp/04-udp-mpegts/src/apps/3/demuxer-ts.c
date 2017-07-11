#include "demuxer-ts.h"


static Logger *lgr = &logger_std;


int demuxer_ts_new(DemuxerTS **out) {
	int ret = 0;
	DemuxerTS *it = NULL;

	it = calloc(1, sizeof(DemuxerTS));
	if (!it) return 1;
	*out = it;

	return ret;
}

int demuxer_ts_init(DemuxerTS *it, URL *u) {
	it->u = *u;
	it->is_psi_printed = 0;
	url_sprint(&it->u, it->us, sizeof(it->us));
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	int ret = 0;
	uint8_t *cursor = NULL;
	MPEGTSHeader ts_hdr = { 0 };
	MPEGTSAdaption ts_adaption = { 0 };
	MPEGTS *ts = NULL;
	char sbuf[8*255] = { 0 };
	DemuxerTS *it = NULL;

	it = (DemuxerTS*)ctx;
	ts = &it->ts;
	cursor = buf;

	if (cursor[0] != MPEGTS_SYNC_BYTE) return 1;

	cursor++;

	mpegts_header_parse(&ts_hdr, cursor); cursor += 3;

	if (ts_hdr.adaption_field_control) {
		mpegts_adaption_parse(&ts_adaption, cursor);
		cursor += 2;

		if (ts_adaption.PCR_flag) {
			// mpegts_pcr_sprint_json(&mpegts_adaption.PCR, sbuf, sizeof(sbuf));
			// log_trace(lgr, "%31s [demux-ts @ %p] %s\n", it->us, ctx, sbuf);
			cursor += 6;
		}
	} else {
		/* PSI-PAT */
		if ((!ts->psi_pat) &&
		    (ts_hdr.PID == MPEGTS_PID_PAT)) {
			cursor++;
			mpegts_psi_pat_del(ts->psi_pat);
			ts->psi_pat = mpegts_psi_pat_new();
			mpegts_psi_pat_parse(ts->psi_pat, cursor);
		}
		/* PSI-PMT */
		else if ((!ts->psi_pmt) &&
		         (ts->psi_pat) &&
		         (ts_hdr.PID == ts->psi_pat->program_map_PID)) {
			cursor++;
			mpegts_psi_pmt_del(ts->psi_pmt);
			ts->psi_pmt = mpegts_psi_pmt_new();
			mpegts_psi_pmt_parse(ts->psi_pmt, cursor);
		}
		else if ((!ts->psi_sdt) &&
		         (ts_hdr.PID == MPEGTS_PID_SDT)) {
			cursor++;
			mpegts_psi_sdt_del(ts->psi_sdt);
			ts->psi_sdt = mpegts_psi_sdt_new();
			mpegts_psi_sdt_parse(ts->psi_sdt, cursor);
		}
	}

	if ((ts->psi_pat) &&
	    (ts->psi_pmt) &&
	    (ts->psi_sdt) &&
	    (!it->is_psi_printed)) {
		mpegts_psi_pat_sprint_humanized(ts->psi_pat, sbuf, sizeof(sbuf));
		log_warn(lgr, "%s %s\n", it->us, sbuf);

		mpegts_psi_sdt_sprint_humanized(ts->psi_sdt, sbuf, sizeof(sbuf));
		log_warn(lgr, "%s %s\n", it->us, sbuf);

		mpegts_psi_pmt_sprint_humanized(ts->psi_pmt, sbuf, sizeof(sbuf));
		log_warn(lgr, "%s %s\n", it->us, sbuf);

		it->is_psi_printed = 1;
	}

	return ret;
}


DemuxerVT demuxer_ts_vt = {
	.consume_pkt_raw = consume_pkt_raw,
};
