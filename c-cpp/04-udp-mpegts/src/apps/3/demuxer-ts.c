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
	it->strm.container_kind = CONTAINER_KIND_MPEGTS;
	url_sprint(&it->u, it->us, sizeof(it->us));

	filter_init(&it->fltr);
	stream_init(&it->strm);
	it->fltr.name = "demuxer-ts";
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

	MPEGTS *ts = NULL;
	MPEGTSHeader ts_hdr = { 0 };
	MPEGTSAdaption ts_adaption = { 0 };
	MPEGTSPES ts_pes = { 0 };

	DemuxerTS *it = NULL;
	Track *trk = NULL;

	it = (DemuxerTS*)ctx;
	ts = &it->ts;
	cursor = buf;

	if (cursor[0] != MPEGTS_SYNC_BYTE) return 1;  /* ERROR: no MPEGTS sync byte found */

	cursor++;

	mpegts_header_parse(&ts_hdr, cursor); cursor += 3;

	if (ts_hdr.adaption_field_control) {
		mpegts_adaption_parse(&ts_adaption, cursor);
		/* 1   => ts_adaption.adaptation_field_length;
		 * ... => adaption payload;
		 */
		cursor += 1 + ts_adaption.adaptation_field_length;
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
		/* PSI-SDT */
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

			if (it->strm.trks.len) {
				log_info(lgr, "input: %s / %s\n", it->us, container_kind_str(it->strm.container_kind));
				{int i = 0; for (i = 0; i < (int)it->strm.trks.len; i++) {
					Track *trk = slice_get(&it->strm.trks, (size_t)i);
					log_info(lgr, "  #%d %3d/0x%04X [%s]\n", trk->i+1, trk->id, trk->id, codec_kind_str(trk->codec_kind));
				}}

				filter_produce_strm(&it->fltr, &it->strm);
				{int i = 0; for (i = 0; i < (int)it->strm.trks.len; i++) {
					Track *trk = slice_get(&it->strm.trks, (size_t)i);
					filter_produce_trk(&it->fltr, trk);
				}}
			}
		}

		/* handle payload */
		if ((ts_hdr.contains_payload) &&
		    (ts_hdr.PID != MPEGTS_PID_PAT) &&
		    (ts_hdr.PID != ts->psi_pat->program_map_PID)) {
			stream_get_track(&it->strm, (uint32_t)ts_hdr.PID, &trk);

			if (trk == NULL) {
				/* log_warn(lgr, "[ts-demuxer @ %s] failed to find track for %d/0x%04X PID inside stream\n",
					it->us, ts_hdr.PID, ts_hdr.PID); */
			} else if (trk->codec_kind == CODEC_KIND_UNKNOWN) {
				/* TODO: detect unknown codec by payload (mp3, subtitles etc) */
			} else {
				if (ts_hdr.payload_unit_start_indicator) {


					/* produce-packet */
					if (trk->pkt.trk)  /* ensure track is set for packet; */
						filter_produce_pkt(&it->fltr, &trk->pkt);

					/*log_info(lgr, "[ts-demuxer @ %s] PID: %d, len: %zu, cap: %zu\n",
						it->us, ts_hdr.PID, trk->pkt.buf.len, trk->pkt.buf.cap);*/
					buf_reset(&trk->pkt.buf);

					if (!mpegts_pes_parse(&ts_pes, cursor)) {
						/* 9   => PES header length;
						 * ... => PTS, DTS, etc
						 */
						cursor += 9 + ts_pes.header_length;

						trk->pkt.trk = trk;
						trk->pkt.PTS = ts_pes.PTS;
						trk->pkt.DTS = ts_pes.DTS;

						buf_write(&trk->pkt.buf, cursor, bufsz - (cursor - buf), NULL);
					} else {
						log_error(lgr, "[ts-demuxer @ %s] PID: %d, len: %zu, cap: %zu\n",
							it->us, ts_hdr.PID, trk->pkt.buf.len, trk->pkt.buf.cap);
					}
				} else {
					if (trk->pkt.trk)  /* ensure track is set for packet; */
						buf_write(&trk->pkt.buf, cursor, bufsz - (cursor - buf), NULL);
				}
			}
		}
	}

cleanup:
	filter_produce_pkt_raw(&it->fltr, buf, bufsz);
	return ret;
}

FilterVT demuxer_ts_filter_vt = {
	.consume_strm = NULL,
	.consume_trk = NULL,
	.consume_pkt = NULL,
	.consume_pkt_raw = consume_pkt_raw,
	.consume_frm = NULL,
};
