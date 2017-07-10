#include "va.h"


static void exec_cb(VAParserWorkerParse *it, void *atom, VAAtomKind atom_kind, uint64_t offset) {
	VAAtomWrapper aw = { 0 };

	if (!it->cb) return;

	aw.id = ++it->id;
	aw.offset = offset;
	aw.kind = atom_kind;
	aw.atom = atom;

	it->cb(it->cb_ctx, &aw);
}

/* TODO: get rid of msg[index]; use meg++ => move pointer instead; */
static void on_msg(VAParserWorkerParse *it, uint8_t *msg) {
	int i = 0;
	MPEGTS *mpegts = &it->mpegts;
	MPEGTSPSIPMTProgramElement *mpegts_psi_pmt_program_element = NULL;
	MPEGTSHeader mpegts_header = { 0 };
	MPEGTSAdaption mpegts_adaption = { 0 };
	MPEGTSPSIPMT mpegts_pmt = { 0 };
	MPEGTSPSI mpegts_psi = { 0 };
	char sbuf[255] = { 0 };

	if (msg[0] != MPEGTS_SYNC_BYTE) return;

	mpegts_header_parse(&mpegts_header, &msg[i+1]);
	/* if (it->cb) it->cb(it->cb_ctx, &mpegts_header, VA_ATOM_KIND_MPEGTS_HEADER, it->offset+1); */
	/* db_store_mpegts_header(it->db, &mpegts_header, it->offset+1); */
	exec_cb(it, &mpegts_header, VA_ATOM_KIND_MPEGTS_HEADER, it->offset+1);

	if (mpegts_header.adaption_field_control) {
		mpegts_adaption_parse(&mpegts_adaption, &msg[i+4]);
		/* if (it->cb) it->cb(it->cb_ctx, &mpegts_adaption, VA_ATOM_KIND_MPEGTS_HEADER, it->offset+4); */
		/* db_store_mpegts_adaption(it->db, &mpegts_adaption, it->offset+4); */
		exec_cb(it, &mpegts_adaption, VA_ATOM_KIND_MPEGTS_HEADER, it->offset+4);

		if (mpegts_adaption.PCR_flag)
			mpegts_pcr_sprint_json(&mpegts_adaption.PCR, sbuf, sizeof(sbuf));
	}

	// PSI-PAT
	if ((!mpegts->psi_pat) &&
	    (mpegts_header.PID == MPEGTS_PID_PAT)) {
		mpegts_psi_pat_del(mpegts->psi_pat);
		mpegts->psi_pat = mpegts_psi_pat_new();
		mpegts_psi_pat_parse(mpegts->psi_pat, &msg[i+5]);
		/* if (it->cb) it->cb(it->cb_ctx, mpegts->psi_pat, VA_ATOM_KIND_MPEGTS_PSI_PAT, it->offset+5); */
		/* db_store_mpegts_psi_pat(it->db, mpegts->psi_pat, it->offset+5); */
		exec_cb(it, mpegts->psi_pat, VA_ATOM_KIND_MPEGTS_PSI_PAT, it->offset+5);

	// PSI-PMT
	} else if ((!mpegts->psi_pmt) &&
	           (mpegts->psi_pat) &&
	           (mpegts_header.PID == mpegts->psi_pat->program_map_PID)) {
		mpegts_psi_pmt_del(mpegts->psi_pmt);
		mpegts->psi_pmt = mpegts_psi_pmt_new();
		mpegts_psi_pmt_parse(mpegts->psi_pmt, &msg[i+5]);
		/* if (it->cb) it->cb(it->cb_ctx, mpegts->psi_pmt, VA_ATOM_KIND_MPEGTS_PSI_PMT, it->offset+5); */
		/* db_store_mpegts_psi_pmt(it->db, mpegts->psi_pmt, it->offset+5); */
		exec_cb(it, mpegts->psi_pmt, VA_ATOM_KIND_MPEGTS_PSI_PMT, it->offset+5);
		mpegts_psi_pmt_program_element = mpegts_psi_pmt_search_by_es_type(mpegts->psi_pmt, MPEGTS_STREAM_TYPE_VIDEO_H264);
		if (mpegts_psi_pmt_program_element != NULL)
			it->video_PID_H264 = mpegts_psi_pmt_program_element->elementary_PID;

	} else if ((!mpegts->psi_sdt) &&
	           (mpegts_header.PID == MPEGTS_PID_SDT)) {
		mpegts_psi_sdt_del(mpegts->psi_sdt);
		mpegts->psi_sdt = mpegts_psi_sdt_new();
		mpegts_psi_sdt_parse(mpegts->psi_sdt, &msg[i+5]);
		/* if (it->cb) it->cb(it->cb_ctx, mpegts->psi_sdt, VA_ATOM_KIND_MPEGTS_PSI_SDT, it->offset+5); */
		/* db_store_mpegts_psi_sdt(it->db, mpegts->psi_sdt, it->offset+5); */
    exec_cb(it, mpegts->psi_sdt, VA_ATOM_KIND_MPEGTS_PSI_SDT, it->offset+5);
	}

	if (mpegts_header.contains_payload) {
		if (mpegts_header.payload_unit_start_indicator) {
			int pes_offset = i + 4;
			if (mpegts_header.adaption_field_control)
				pes_offset = pes_offset + mpegts_adaption.adaptation_field_length + 1;

			MPEGTSPES mpegts_pes = { 0 };
			if (!mpegts_pes_parse(&mpegts_pes, &msg[pes_offset])) {
				/* mpegts_pes_print_humanized(&mpegts_pes); */
				/* mpegts_pes_print_json(&mpegts_pes); */

				/* db_store_mpegts_pes(it->db, &mpegts_pes, it->offset + pes_offset); */
				/* if (it->cb) it->cb(it->cb_ctx, &mpegts_pes, VA_ATOM_KIND_MPEGTS_PES, it->offset + pes_offset); */
				exec_cb(it, &mpegts_pes, VA_ATOM_KIND_MPEGTS_PES, it->offset + pes_offset);

				uint8_t *es_data = &msg[pes_offset + 9 + mpegts_pes.header_length];
				int es_offset = pes_offset + 9 + mpegts_pes.header_length;
				int es_length = MPEGTS_PACKET_SIZE - es_offset;

				if (mpegts_header.PID == it->video_PID_H264) {
					if (!it->h264_annex_b_parse_result) {
						/* TODO: move to new_H264AnnexBParseResult() */
						it->h264_annex_b_parse_result = calloc(1, sizeof(H264AnnexBParseResult));
						it->h264_annex_b_parse_result->len = 0;
					} else { /* exec-cb; always emit previous result; */
						if (it->h264_annex_b_parse_result->len) {
							h264_annexb_parse_result_print_humanized_one_line(it->h264_annex_b_parse_result);

							{int ii = 0; for (ii = 0; ii < it->h264_annex_b_parse_result->len; ii++) {
								H264NAL *nal = &it->h264_annex_b_parse_result->nals[ii];
								uint64_t nal_offset = it->h264_annex_b_parse_result->offsets[ii];

								/* db_store_h264(it->db, nal, nal_offset); */
								/* if (it->cb) it->cb(it->cb_ctx, nal, va_atom_kind_from_h264_nal_type(nal->type), nal_offset); */
								exec_cb(it, nal, va_atom_kind_from_h264_nal_type(nal->type), nal_offset);
							}}
						}

						memset(it->h264_annex_b_parse_result, 0, sizeof(H264AnnexBParseResult));
					}

					h264_annexb_parse(
						&it->h264,
						&msg[es_offset],
						(size_t)es_length,
						it->offset + (uint64_t)es_offset,
						it->h264_annex_b_parse_result
					);
				}
			}
		} else {
			int es_offset = i + 4;
			if (mpegts_header.adaption_field_control)
				es_offset = es_offset + mpegts_adaption.adaptation_field_length + 1;
			int es_length = MPEGTS_PACKET_SIZE - es_offset;

			if (mpegts_header.PID == it->video_PID_H264) {
				/* NAL size determination */
				h264_annexb_parse(
					&it->h264,
					&msg[es_offset],
					(size_t)es_length,
					it->offset + (uint64_t)es_offset,
					it->h264_annex_b_parse_result
				);

				if ((it->h264_annex_b_parse_result) && (it->h264_annex_b_parse_result->len)) {
					it->h264_annex_b_parse_result->nals[it->h264_annex_b_parse_result->len-1].sz += (size_t)es_length;
				}
			}
		}
	}

cleanup:
	it->offset += MPEGTS_PACKET_SIZE;
}


void* parser_worker_parse_do(void *args) {
	int i = 0;
	size_t readed_len = 0,
	       fifo_length = 0;
	VAParserWorkerParse *it = (VAParserWorkerParse*)args;

	for(;;) {
		fifo_wait_data(it->fifo);
		fifo_length = fifo_len(it->fifo);

		uint8_t msg[MPEGTS_PACKET_SIZE] = { 0 };

		for (i = 0; i < fifo_length; i += MPEGTS_PACKET_SIZE) {
			fifo_read(it->fifo, msg, MPEGTS_PACKET_SIZE, &readed_len);
			on_msg(it, msg);
		}
	}
}
