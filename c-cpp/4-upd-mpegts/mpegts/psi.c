#include "mpegts.h"


const char* mpegts_PED_tag_string(MPEGTSPEDTag it) {
	switch(it) {
	case MPEGTS_PED_TAG_RESERVED_00: return MPEGTS_PED_TAG_RESERVED_00_STR;
	case MPEGTS_PED_TAG_RESERVED_01: return MPEGTS_PED_TAG_RESERVED_01_STR;
	case MPEGTS_PED_TAG_ISO_639: return MPEGTS_PED_TAG_ISO_639_STR;
	}

	return "unknown Program Element Descriptor tag";
}

void mpegts_psi_parse(MPEGTSPSI *it, uint8_t *data) {
	// PSI - header
	uint8_t table_id = (uint8_t)data[0];
	uint8_t section_syntax_indicator = !!( (uint8_t)data[1] & 0x80 );
	uint8_t private_bit = !!( (uint8_t)data[1] & 0x40 );
	uint8_t reserved_bits = (uint8_t)data[1] & 0x30;
	uint8_t section_length_unused_bits = (uint8_t)data[1] & 0x0C;
	uint16_t section_length = (((uint16_t)data[1] & 0x03 ) << 8) | ((uint16_t)data[2] & 0xFF);

	// PSI - table syntax section
	uint16_t transport_stream_id = (
		(((uint16_t)data[3] & 0xFF) << 8) |
		((uint16_t)data[4] & 0xFF)
	);
	uint8_t version_number = (uint8_t)data[5] & 0x3E;
	uint8_t curent_next_indicator = (uint8_t)data[5] & 0x01;
	uint8_t section_number = (uint8_t)data[6];
	uint8_t last_section_number = (uint8_t)data[7];

	// 3 bytes of PSI header
	// [0, 1, 2, ..., -3, -2, -1, -0]
	// e.g. section-length = 5;
	//      [0, 1, 2, 3  , 4      , 5      , 6      , 7]
	//      [header, ...
	//          ..., data, crc32-0, crc32-1, crc32-2, crc32-3]
	//      => section starts at data[3] where 3 is 2 + 1
	//      => crc32-i = 7 = 2 + section-length
	uint16_t CRC32_i = 2 + section_length;
	uint32_t CRC32 = (
		(((uint32_t)data[CRC32_i-3] & 0xFF) << 24) |
		(((uint32_t)data[CRC32_i-2] & 0xFF) << 16) |
		(((uint32_t)data[CRC32_i-1] & 0xFF) << 8) |
		((uint32_t)data[CRC32_i] & 0xFF)
	);

	it->table_id = table_id;
	it->section_syntax_indicator = section_syntax_indicator;
	it->private_bit = private_bit;
	it->reserved_bits = reserved_bits;
	it->section_length_unused_bits = section_length_unused_bits;
	it->section_length = section_length;

	it->transport_stream_id = transport_stream_id;
	it->version_number = version_number;
	it->curent_next_indicator = curent_next_indicator;
	it->section_number = section_number;
	it->last_section_number = last_section_number;

	it->CRC32 = CRC32;
}

void mpegts_psi_print_json(MPEGTSPSI *it) {
	printf(
		"{\"table-id\": %d (0x%02x)"
		", \"section-syntax-indicator\": %d"
		", \"private-bit\": %d"
		", \"section-length\": %d"
		", \"transport-stream-id\": %d"
		", \"version-number\": %d"
		", \"curent-next-indicator\": %d"
		", \"section-number\": %d"
		", \"last-section-number\": %d"
		", \"CRC32\": 0x%08X"
		"}\n",
		it->table_id, it->table_id,
		it->section_syntax_indicator,
		it->private_bit,
		it->section_length,
		it->transport_stream_id,
		it->version_number,
		it->curent_next_indicator,
		it->section_number,
		it->last_section_number,
		it->CRC32
	);
}

void mpegts_pat_parse(MPEGTSPAT *it, uint8_t *data) {
	it->program_number = (((uint16_t)data[0] & 0xFF) << 8) | ((uint16_t)data[1] & 0xFF);
	it->program_map_PID = (((uint16_t)data[2] & 0x1F) << 8) | ((uint16_t)data[3] & 0xFF);
}

void mpegts_pat_print_json(MPEGTSPAT *it) {
	printf(
		"{\"program-number\": %d"
		", \"program-map-PID\": %d"
		"}\n",
		it->program_number,
		it->program_map_PID
	);
}

void mpegts_pmt_parse(MPEGTSPMT *it, MPEGTSPSI *psi, uint8_t *data) {
	int pos = 0, // parser position current
	    end = 0; // finish

	int es_info_pos = 0, // ES-info => parser position current
	    es_info_end = 0; // ES-info => finish
	uint8_t *es_info_data = NULL;

	end = (int)psi->section_length;
	// transport-stream-id   x2
	// version-number        x1
	// curent-next-indicator
	// section-number        x1
	// last-section-number   x1
	// CRC32                 x4
	//
	// -----                 x9
	end -= 9;

	it->PCR_PID = (
		(((uint16_t)data[pos++] & 0x1F) << 8) |
		 ((uint16_t)data[pos++] & 0xFF)
	);

	it->program_info_length = (
		(((uint16_t)data[pos++] & 0x03) << 8) |
		 ((uint16_t)data[pos++] & 0xFF)
	);

	while (pos < end) {
		uint8_t stream_type = (uint8_t)data[pos++];
		uint16_t elementary_PID = (
			(((uint16_t)data[pos++] & 0x1F) << 8) |
			((uint16_t)data[pos++] & 0xFF)
		);
		uint16_t ES_info_length = (
			(((uint16_t)data[pos++] & 0x03) << 8) |
			((uint16_t)data[pos++] & 0xFF)
		);

		if (ES_info_length) {
			es_info_pos = 0;
			es_info_end = (int)ES_info_length;
			es_info_data = &data[pos];

			while (es_info_pos < es_info_end) {
				MPEGTSPEDTag descriptor_tag = (MPEGTSPEDTag)es_info_data[es_info_pos++];
				uint8_t descriptor_length = (uint8_t)es_info_data[es_info_pos++];
				es_info_pos += (int)descriptor_length;
				printf("~~~ (%X \"%s\") %d\n", descriptor_tag, mpegts_PED_tag_string(descriptor_tag), descriptor_length);
			}
		}

		pos += (int)ES_info_length;

		printf("~~~ %X %s\n", elementary_PID, mpegts_es_type_string(stream_type));
	}

	// int pmt_start = i + 17;
	// int pmt_offset = 0;
	// while (pos <= end) {
	// 	uint8_t stream_type = (uint8_t)msg[pmt_start+pmt_offset];
	// 	uint16_t elementary_PID = (
	// 		(((uint16_t)msg[pmt_start+pmt_offset+1] & 0x1F) << 8) |
	// 		((uint16_t)msg[pmt_start+pmt_offset+2] & 0xFF)
	// 	);
	// 	uint16_t ES_info_length = (
	// 		(((uint16_t)msg[pmt_start+pmt_offset+3] & 0x03) << 8) |
	// 		((uint16_t)msg[pmt_start+pmt_offset+4] & 0xFF)
	// 	);

	// 	printf("%s\n", mpegts_es_type_string(stream_type));
	// 	// printf("\t - 0x%02x | 0x%02x | %d | %d | %s\n", stream_type, elementary_PID, ES_info_length, psi.section_length, stream_type_name);

	// 	int ES_info_start = pmt_start+pmt_offset+5;
	// 	int ES_info_offset = 0;
	// 	int16_t ES_info_length_unreaded = ES_info_length;
	// 	while (ES_info_length_unreaded > 0) {
	// 		uint8_t descriptor_tag = (uint8_t)msg[ES_info_start+ES_info_offset];
	// 		uint8_t descriptor_length = (uint8_t)msg[ES_info_start+ES_info_offset+1];
	// 		printf("\t\t - 0x%02x | %d\n", descriptor_tag, descriptor_length);
	// 		ES_info_length_unreaded -= (2 + descriptor_length);
	// 		descriptor_length += (2 + descriptor_length);
	// 	}

	// 	section_length_unreaded -= (5 + ES_info_length);
	// 	pmt_offset +=	(5 + ES_info_length);

	// 	if (stream_type == MPEGTS_STREAM_TYPE_VIDEO_H264)
	// 		app->video_PID_H264 = elementary_PID;
	// }
}

void mpegts_pmt_print_json(MPEGTSPMT *it) {

}