#include "mpegts.h"


// http://dvd.sourceforge.net/dvdinfo/pes-hdr.html
int mpegts_pes_parse(MPEGTSPES *it, uint8_t *data) {
	uint32_t start_code = (
		0 << 24                 |
		(uint32_t)data[0] << 16 |
		(uint32_t)data[1] << 8  |
		(uint32_t)data[2]
	);

	if (start_code != MPEGTS_PES_START_CODE) return 1;

	it->stream_id = (uint8_t)data[3];
	it->packet_length = (
		(uint16_t)data[4] & 0xFF << 8 |
		(uint16_t)data[5] & 0xFF
	);

	// PES header
	// 10 binary or 0x2 hex
	it->marker_bits = (((uint8_t)data[6] & 0xC0) >> 6);
	// 00 - not scrambled
	it->scrambling_control = (uint8_t)data[6] & 0x30;
	it->priority = !!( (uint8_t)data[6] & 0x08 );
	// 1 indicates that the PES packet
	// header is immediately followed by
	// the video start code or audio syncword
	it->data_alignment_indicator = !!( (uint8_t)data[6] & 0x04 );
	it->copyright = !!( (uint8_t)data[6] & 0x02 );
	it->original_or_copy = !!( (uint8_t)data[6] & 0x01 );

	// 11 = both present;
	// 01 = forbidden;
	// 10 = only PTS;
	// 00 = no PTS or DTS
	it->PTS_DTS_indicator = (((uint8_t)data[7] & 0xC0) >> 6);
	// This is the Elementary Stream Clock Reference,
	// used if the stream and system levels are not synchronized'
	// (i.e. ESCR differs from SCR in the PACK header).
	it->ESCR_flag = (uint8_t)data[7] & 0x20;
	// The rate at which data is delivered for this stream,
	// in units of 50 bytes/second.
	it->ES_rate_flag = (uint8_t)data[7] & 0x10;
	it->DSM_trick_mode_flag = (uint8_t)data[7] & 0x08;
	it->additional_copy_info_flag = (uint8_t)data[7] & 0x04;
	it->CRC_flag = (uint8_t)data[7] & 0x02;
	it->extension_flag = (uint8_t)data[7] & 0x01;

	it->header_length = (uint8_t)data[8];

	if ((it->PTS_DTS_indicator == MPEGTS_PES_PTS_DTS_INDICATOR_PTS) ||
		  (it->PTS_DTS_indicator == MPEGTS_PES_PTS_DTS_INDICATOR_PTS_DTS)) {
 		it->PTS = (
			(((uint64_t)data[9]  & 0x0E) << 29) | // (>> 1 (<< 30))
			(((uint64_t)data[10] & 0xFF) << 22) |
			(((uint64_t)data[11] & 0xFE) << 14) | // (>> 1 (<< 15))
			(((uint64_t)data[12] & 0xFF) << 7)  |
			 ((uint64_t)data[13] & 0xFE) >> 1
		);
		it->DTS = it->PTS;

		if (it->PTS_DTS_indicator == MPEGTS_PES_PTS_DTS_INDICATOR_PTS_DTS) {
			it->DTS = (
				(((uint64_t)data[14]  & 0x0E) << 29) | // (>> 1 (<< 30))
				(((uint64_t)data[15] & 0xFF) << 22)  |
				(((uint64_t)data[16] & 0xFE) << 14)  | // (>> 1 (<< 15))
				(((uint64_t)data[17] & 0xFF) << 7)   |
				 ((uint64_t)data[18] & 0xFE) >> 1
			);
		}
	}

	return 0;
}

inline static const char *mpegts_pes_scrambling_control_string(uint8_t v) {
	if (v == 0x00) return "not scrambled";
	return "user defined";
}

inline static const char *mpegts_pes_pts_dts_indicator_string(uint8_t v) {
	switch (v) {
	case MPEGTS_PES_PTS_DTS_INDICATOR_NO: return "no PTS or DTS data present";
	case MPEGTS_PES_PTS_DTS_INDICATOR_FORBIDDEN: return "forbidden";
	case MPEGTS_PES_PTS_DTS_INDICATOR_PTS: return "PTS";
	case MPEGTS_PES_PTS_DTS_INDICATOR_PTS_DTS: return "PTS and DTS";
	}
}

void mpegts_pes_print_json(MPEGTSPES *it) {
	printf(
		"{"
			"\"stream-id\": 0x%02X"
			", \"packet-length\": %d"
			", \"PES-scrambling-control\": %d"
			", \"PES-priority\": %d"
			", \"data-alignment-indicator\": %d"
			", \"copyright\": %d"
			", \"original-or-copy\": %d"
			", \"PTS-DTS-indicator\": %d"
			", \"ESCR-flag\": %d"
			", \"ES-rate-flag\": %d"
			", \"DSM-trick-mode-flag\": %d"
			", \"additional-copy-info-flag\": %d"
			", \"PES-CRC-flag\": %d"
			", \"PES-extension-flag\": %d"
			", \"PES-header-data-length\": %d"
			", \"PTS\": %" PRIu64 ""
			", \"DTS\": %" PRIu64 ""
		"}\n",
		it->stream_id,
		it->packet_length,
		it->scrambling_control,
		it->priority,
		it->data_alignment_indicator,
		it->copyright,
		it->original_or_copy,
		it->PTS_DTS_indicator,
		it->ESCR_flag,
		it->ES_rate_flag,
		it->DSM_trick_mode_flag,
		it->additional_copy_info_flag,
		it->CRC_flag,
		it->extension_flag,
		it->header_length,
		it->PTS,
		it->DTS
	);
}

void mpegts_pes_print_humanized(MPEGTSPES *it) {
	printf("PES packet:\n");
	printf("  stream-id: %d (0x%02X)\n", it->stream_id, it->stream_id);
	printf("  packet-length: %d (0x%02X)\n", it->packet_length, it->packet_length);
	printf("  PES-scrambling-control: %d (0x%02X) / \"%s\"\n", it->scrambling_control, it->scrambling_control, mpegts_pes_scrambling_control_string(it->scrambling_control));
	printf("  PES-priority: %d (0x%02X)\n", it->priority, it->priority);
	printf("  data-alignment-indicator: %d (0x%02X)\n", it->data_alignment_indicator, it->data_alignment_indicator);
	printf("  copyright: %d (0x%02X)\n", it->copyright, it->copyright);
	printf("  original-or-copy: %d (0x%02X)\n", it->original_or_copy, it->original_or_copy);
	printf("  PTS-DTS-indicator: %d (0x%02X) / \"%s\"\n", it->PTS_DTS_indicator, it->PTS_DTS_indicator, mpegts_pes_pts_dts_indicator_string(it->PTS_DTS_indicator));
	printf("  ESCR-flag: %d (0x%02X)\n", it->ESCR_flag, it->ESCR_flag);
	printf("  ES-rate-flag: %d (0x%02X)\n", it->ES_rate_flag, it->ES_rate_flag);
	printf("  DSM-trick-mode-flag: %d (0x%02X)\n", it->DSM_trick_mode_flag, it->DSM_trick_mode_flag);
	printf("  additional-copy-info-flag: %d (0x%02X)\n", it->additional_copy_info_flag, it->additional_copy_info_flag);
	printf("  PES-CRC-flag: %d (0x%02X)\n", it->CRC_flag, it->CRC_flag);
	printf("  PES-extension-flag: %d (0x%02X)\n", it->extension_flag, it->extension_flag);
	printf("  PES-header-data-length: %d (0x%02X)\n", it->header_length, it->header_length);
	if (it->PTS)
		printf("    PTS: \"0:0:0:XXX (%" PRIu64 ")\"\n", it->PTS);
	if (it->DTS)
		printf("    DTS: \"0:0:0:XXX (%" PRIu64 ")\"\n", it->DTS);
	if ((it->PTS) && (it->DTS))
		printf("    PTS - DTS: %" PRId64 "\n", it->PTS - it->DTS);
}
