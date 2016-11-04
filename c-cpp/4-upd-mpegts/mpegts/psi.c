#include "mpegts.h"


static void                     mpegts_pmt_program_element_parse_header(MPEGTSPMTProgramElement *it, uint8_t *data);
static void                     mpegts_pmt_program_element_parse_es_infos(MPEGTSPMTProgramElement *it, uint8_t *data);
static MPEGTSPMTProgramElement* mpegts_pmt_program_elements_push_unique(MPEGTSPMTProgramElements *it, MPEGTSPMTProgramElement *b);
static void                     mpegts_pmt_es_info_parse(MPEGTSPMTESInfo *it, uint8_t *data);
static MPEGTSPMTESInfo*         mpegts_pmt_es_infos_push_unique(MPEGTSPMTESInfos *it, MPEGTSPMTESInfo *b);


const char* mpegts_PED_tag_string(MPEGTSPEDTag it) {
	switch(it) {
	case MPEGTS_PED_TAG_RESERVED_00:                   return MPEGTS_PED_TAG_RESERVED_00_STR;
	case MPEGTS_PED_TAG_RESERVED_01:                   return MPEGTS_PED_TAG_RESERVED_01_STR;
	case MPEGTS_PED_TAG_V_H262_13818_11172:            return MPEGTS_PED_TAG_V_H262_13818_11172_STR;
	case MPEGTS_PED_TAG_A_13818_11172:                 return MPEGTS_PED_TAG_A_13818_11172_STR;
	case MPEGTS_PED_TAG_HIERARCHY:                     return MPEGTS_PED_TAG_HIERARCHY_STR;
	case MPEGTS_PED_TAG_REG_PRIVATE:                   return MPEGTS_PED_TAG_REG_PRIVATE_STR;
	case MPEGTS_PED_TAG_DATA_STREAM_ALIGN:             return MPEGTS_PED_TAG_DATA_STREAM_ALIGN_STR;
	case MPEGTS_PED_TAG_GRID:                          return MPEGTS_PED_TAG_GRID_STR;
	case MPEGTS_PED_TAG_VIDEO_WINDOW:                  return MPEGTS_PED_TAG_VIDEO_WINDOW_STR;
	case MPEGTS_PED_TAG_CAS_EMM_ECM_PID:               return MPEGTS_PED_TAG_CAS_EMM_ECM_PID_STR;
	case MPEGTS_PED_TAG_ISO_639:                       return MPEGTS_PED_TAG_ISO_639_STR;
	case MPEGTS_PED_TAG_SYSTEM_CLOCK_EXT_REF:          return MPEGTS_PED_TAG_SYSTEM_CLOCK_EXT_REF_STR;
	case MPEGTS_PED_TAG_MULT_BUF_UTIL_BOUNDS:          return MPEGTS_PED_TAG_MULT_BUF_UTIL_BOUNDS_STR;
	case MPEGTS_PED_TAG_COPYRIGHT:                     return MPEGTS_PED_TAG_COPYRIGHT_STR;
	case MPEGTS_PED_TAG_MAX_BIT_RATE:                  return MPEGTS_PED_TAG_MAX_BIT_RATE_STR;
	case MPEGTS_PED_TAG_PRIVATE_DATA_INDICATOR:        return MPEGTS_PED_TAG_PRIVATE_DATA_INDICATOR_STR;
	case MPEGTS_PED_TAG_SMOOTHING_BUFFER:              return MPEGTS_PED_TAG_SMOOTHING_BUFFER_STR;
	case MPEGTS_PED_TAG_STD_VIDEO_BUFFER_LEAK_CONTROL: return MPEGTS_PED_TAG_STD_VIDEO_BUFFER_LEAK_CONTROL_STR;
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
	int pos = 0, // program-element => parser position current
	    end = 0; // program-element => finish

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
		MPEGTSPMTProgramElement b = { 0 },
		                       *a = NULL;
		mpegts_pmt_program_element_parse_header(&b, &data[pos]);
		a = mpegts_pmt_program_elements_push_unique(&it->program_elements, &b);
		mpegts_pmt_program_element_parse_es_infos(a, &data[pos+5]);

		// stream_type    x1;
		// elementary_PID x2;
		// ES_info_length x2;
		pos += 5;
		pos += (int)a->ES_info_length;
	}
}

static void mpegts_pmt_program_element_parse_header(MPEGTSPMTProgramElement *it, uint8_t *data) {
	int pos = 0, // ES-info => parser position current
	    end = 0; // ES-info => finish
	uint8_t *es_info_data = NULL;

	it->stream_type = (uint8_t)data[0];
	it->elementary_PID = (
		(((uint16_t)data[1] & 0x1F) << 8) |
		((uint16_t)data[2] & 0xFF)
	);
	it->ES_info_length = (
		(((uint16_t)data[3] & 0x03) << 8) |
		((uint16_t)data[4] & 0xFF)
	);
}

static void mpegts_pmt_program_element_parse_es_infos(MPEGTSPMTProgramElement *it, uint8_t *data) {
	int pos = 0, // ES-info => parser position current
	    end = 0; // ES-info => finish

	printf("~~~ %X %s\n", it->elementary_PID, mpegts_es_type_string(it->stream_type));

	if (it->ES_info_length) {
		pos = 0;
		end = (int)it->ES_info_length;

		while (pos < end) {
			MPEGTSPMTESInfo es_info = { 0 };
			mpegts_pmt_es_info_parse(&es_info, &data[pos]);

			mpegts_pmt_es_infos_push_unique(&it->es_infos, &es_info);

			// descriptor_tag    x1;
			// descriptor_length x1;
			pos += 2;
			pos += (int)es_info.descriptor_length;
		}
	}
}

void mpegts_pmt_es_info_parse(MPEGTSPMTESInfo *it, uint8_t *data) {
	it->descriptor_tag = (MPEGTSPEDTag)data[0];
	it->descriptor_length = (uint8_t)data[1];
	printf("~~~ \t (%X \"%s\") %d\n", it->descriptor_tag, mpegts_PED_tag_string(it->descriptor_tag), it->descriptor_length);
	if (it->descriptor_length)
		memcpy(
			it->descriptor_data, // dst
			&data[2],            // src
			sizeof(it->descriptor_data) <= (size_t)it->descriptor_length
				? (size_t)it->descriptor_length
				: sizeof(it->descriptor_data)
		);

	if (it->descriptor_length) {
		int i;
		printf("~~~ \t ");
		for (i=0; i < (int)it->descriptor_length; i++) {
			printf("%c", it->descriptor_data[i], mpegts_PED_tag_string(it->descriptor_tag), it->descriptor_length);
		}
		printf("\n");
	}
}

static MPEGTSPMTProgramElement* mpegts_pmt_program_elements_push_unique(MPEGTSPMTProgramElements *it, MPEGTSPMTProgramElement *b) {
	int i = 0;
	MPEGTSPMTProgramElement *a;

	if ((!it) || (!b)) return;

	for (i = 0; i < (int)it->len; i++) {
		a = &it->c[i];

		if (a->elementary_PID == b->elementary_PID) return a;
	}

	if (it->cap == 0) {
		it->c = (MPEGTSPMTProgramElement*)calloc(
			1,
			sizeof(MPEGTSPMTProgramElement));
	} else {
		it->c = (MPEGTSPMTProgramElement*)realloc(
			it->c,
			(it->cap + 1)*sizeof(MPEGTSPMTProgramElement));
	}
	it->cap++;
	it->len++;

	a = &it->c[it->len-1];
	*a = *b;

	return a;
}

static MPEGTSPMTESInfo* mpegts_pmt_es_infos_push_unique(MPEGTSPMTESInfos *it, MPEGTSPMTESInfo *b) {
	int i = 0;
	MPEGTSPMTESInfo *a;

	if ((!it) || (!b)) return;

	for (i = 0; i < (int)it->len; i++) {
		a = &it->c[i];

		if (a->descriptor_tag == b->descriptor_tag) return a;
	}

	if (it->cap == 0) {
		it->c = (MPEGTSPMTESInfo*)calloc(
			1,
			sizeof(MPEGTSPMTESInfo));
	} else {
		it->c = (MPEGTSPMTESInfo*)realloc(
			it->c,
			(it->cap + 1)*sizeof(MPEGTSPMTESInfo));
	}
	it->cap++;
	it->len++;

	a = &it->c[it->len-1];
	*a = *b;

	return a;
}

void mpegts_pmt_print_json(MPEGTSPMT *it) {

}
