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
	it->is_psi_logged = 0;
	it->is_stream_builded = 0;
	url_sprint(&it->u, it->us, sizeof(it->us));
}

/* log down PSI tables */
static void log_psi(DemuxerTS *it) {
	char buf[8*255] = { 0 };
	MPEGTS *ts = NULL;

	ts = &it->ts;

	if (it->is_psi_logged) return;

	mpegts_psi_pat_sprint_humanized(ts->psi_pat, buf, sizeof(buf));
	log_trace(lgr, "%s %s\n", it->us, buf);

	mpegts_psi_sdt_sprint_humanized(ts->psi_sdt, buf, sizeof(buf));
	log_trace(lgr, "%s %s\n", it->us, buf);

	mpegts_psi_pmt_sprint_humanized(ts->psi_pmt, buf, sizeof(buf));
	log_trace(lgr, "%s %s\n", it->us, buf);

	it->is_psi_logged = 1;
}

/* build stream from PSI PMT table */
static void build_stream(DemuxerTS *it) {
	if (it->is_stream_builded) return;

	stream_from_mpegts_psi_pmt(&it->strm, it->ts.psi_pmt);

	it->is_stream_builded = 1;
}

static int consume_pkt_raw(void *ctx, uint8_t *buf, size_t bufsz) {
	int ret = 0;
	uint8_t *cursor = NULL;
	MPEGTSHeader ts_hdr = { 0 };
	MPEGTSAdaption ts_adaption = { 0 };
	MPEGTS *ts = NULL;
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

		if (ts_adaption.PCR_flag) cursor += 6;
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
	    (ts->psi_sdt)) {
		// if (!it->is_psi_logged) log_psi(it);
		if (!it->is_stream_builded) {
			build_stream(it);

			if (it->strm.trks) {
				log_info(lgr, "input: %s\n", it->us);
				{int i = 0; for (i = 0; i < (int)it->strm.trks->len; i++) {
					Track *trk = slice_get(it->strm.trks, (size_t)i);
					log_info(lgr, "  #%d %3d/0x%04X [%s]\n", trk->i+1, trk->id, trk->id, codec_kind_str(trk->codec_kind));
				}}
			}
		}
	}

	if (ts_hdr.contains_payload) {
		if (ts_hdr.payload_unit_start_indicator) {
		} else {
		}
	}

	return ret;
}


DemuxerVT demuxer_ts_vt = {
	.consume_pkt_raw = consume_pkt_raw,
};
