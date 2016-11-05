#include "mpegts.h"


static void                        mpegts_psi_pmt_program_element_parse_header(MPEGTSPSIPMTProgramElement *it, uint8_t *data);
static void                        mpegts_psi_pmt_program_element_parse_es_infos(MPEGTSPSIPMTProgramElement *it, uint8_t *data);
static MPEGTSPSIPMTProgramElement* mpegts_psi_pmt_program_elements_push_unique(MPEGTSPSIPMTProgramElements *it, MPEGTSPSIPMTProgramElement *b);
static void                        mpegts_psi_pmt_es_info_parse(MPEGTSPSIPMTESInfo *it, uint8_t *data);
static MPEGTSPSIPMTESInfo*         mpegts_psi_pmt_es_infos_push_unique(MPEGTSPSIPMTESInfos *it, MPEGTSPSIPMTESInfo *b);


MPEGTSPSIPMT *mpegts_psi_pmt_new(void) {
	MPEGTSPSIPMT *it = calloc(1, sizeof(MPEGTSPSIPMT));
	return it;
}

const char* mpegts_psi_ped_tag_string(MPEGTSPSIPEDTag it) {
	switch(it) {
	case MPEGTS_PSI_PED_TAG_RESERVED_00:                   return MPEGTS_PSI_PED_TAG_RESERVED_00_STR;
	case MPEGTS_PSI_PED_TAG_RESERVED_01:                   return MPEGTS_PSI_PED_TAG_RESERVED_01_STR;
	case MPEGTS_PSI_PED_TAG_V_H262_13818_11172:            return MPEGTS_PSI_PED_TAG_V_H262_13818_11172_STR;
	case MPEGTS_PSI_PED_TAG_A_13818_11172:                 return MPEGTS_PSI_PED_TAG_A_13818_11172_STR;
	case MPEGTS_PSI_PED_TAG_HIERARCHY:                     return MPEGTS_PSI_PED_TAG_HIERARCHY_STR;
	case MPEGTS_PSI_PED_TAG_REG_PRIVATE:                   return MPEGTS_PSI_PED_TAG_REG_PRIVATE_STR;
	case MPEGTS_PSI_PED_TAG_DATA_STREAM_ALIGN:             return MPEGTS_PSI_PED_TAG_DATA_STREAM_ALIGN_STR;
	case MPEGTS_PSI_PED_TAG_GRID:                          return MPEGTS_PSI_PED_TAG_GRID_STR;
	case MPEGTS_PSI_PED_TAG_VIDEO_WINDOW:                  return MPEGTS_PSI_PED_TAG_VIDEO_WINDOW_STR;
	case MPEGTS_PSI_PED_TAG_CAS_EMM_ECM_PID:               return MPEGTS_PSI_PED_TAG_CAS_EMM_ECM_PID_STR;
	case MPEGTS_PSI_PED_TAG_ISO_639:                       return MPEGTS_PSI_PED_TAG_ISO_639_STR;
	case MPEGTS_PSI_PED_TAG_SYSTEM_CLOCK_EXT_REF:          return MPEGTS_PSI_PED_TAG_SYSTEM_CLOCK_EXT_REF_STR;
	case MPEGTS_PSI_PED_TAG_MULT_BUF_UTIL_BOUNDS:          return MPEGTS_PSI_PED_TAG_MULT_BUF_UTIL_BOUNDS_STR;
	case MPEGTS_PSI_PED_TAG_COPYRIGHT:                     return MPEGTS_PSI_PED_TAG_COPYRIGHT_STR;
	case MPEGTS_PSI_PED_TAG_MAX_BIT_RATE:                  return MPEGTS_PSI_PED_TAG_MAX_BIT_RATE_STR;
	case MPEGTS_PSI_PED_TAG_PRIVATE_DATA_INDICATOR:        return MPEGTS_PSI_PED_TAG_PRIVATE_DATA_INDICATOR_STR;
	case MPEGTS_PSI_PED_TAG_SMOOTHING_BUFFER:              return MPEGTS_PSI_PED_TAG_SMOOTHING_BUFFER_STR;
	case MPEGTS_PSI_PED_TAG_STD_VIDEO_BUFFER_LEAK_CONTROL: return MPEGTS_PSI_PED_TAG_STD_VIDEO_BUFFER_LEAK_CONTROL_STR;
	}

	return "unknown Program Element Descriptor tag";
}

void mpegts_psi_pmt_parse(MPEGTSPSIPMT *it, uint8_t *data) {
	int pos = 0, // program-element => parser position current
	    end = 0; // program-element => finish

	mpegts_psi_parse(&it->psi, data);

	if (it->psi.table_id != MPEGTS_TABLE_ID_PROGRAM_MAP_SECTION) return;

	// psi-size x8
	data += 8;

	end = (int)it->psi.section_length;
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
		MPEGTSPSIPMTProgramElement b = { 0 },
		                          *a = NULL;
		mpegts_psi_pmt_program_element_parse_header(&b, &data[pos]);
		a = mpegts_psi_pmt_program_elements_push_unique(&it->program_elements, &b);
		mpegts_psi_pmt_program_element_parse_es_infos(a, &data[pos+5]);

		// stream_type    x1;
		// elementary_PID x2;
		// ES_info_length x2;
		pos += 5;
		pos += (int)a->ES_info_length;
	}
}

static void mpegts_psi_pmt_program_element_parse_header(MPEGTSPSIPMTProgramElement *it, uint8_t *data) {
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

static void mpegts_psi_pmt_program_element_parse_es_infos(MPEGTSPSIPMTProgramElement *it, uint8_t *data) {
	int pos = 0, // ES-info => parser position current
	    end = 0; // ES-info => finish

	printf("~~~ %X %s\n", it->elementary_PID, mpegts_es_type_string(it->stream_type));

	if (it->ES_info_length) {
		pos = 0;
		end = (int)it->ES_info_length;

		while (pos < end) {
			MPEGTSPSIPMTESInfo es_info = { 0 };
			mpegts_psi_pmt_es_info_parse(&es_info, &data[pos]);

			mpegts_psi_pmt_es_infos_push_unique(&it->es_infos, &es_info);

			// descriptor_tag    x1;
			// descriptor_length x1;
			pos += 2;
			pos += (int)es_info.descriptor_length;
		}
	}
}

void mpegts_psi_pmt_es_info_parse(MPEGTSPSIPMTESInfo *it, uint8_t *data) {
	it->descriptor_tag = (MPEGTSPSIPEDTag)data[0];
	it->descriptor_length = (uint8_t)data[1];
	printf("~~~ \t (%X \"%s\") %d\n", it->descriptor_tag, mpegts_psi_ped_tag_string(it->descriptor_tag), it->descriptor_length);
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
			printf("%c", it->descriptor_data[i], mpegts_psi_ped_tag_string(it->descriptor_tag), it->descriptor_length);
		}
		printf("\n");
	}
}

static MPEGTSPSIPMTProgramElement* mpegts_psi_pmt_program_elements_push_unique(MPEGTSPSIPMTProgramElements *it, MPEGTSPSIPMTProgramElement *b) {
	int i = 0;
	MPEGTSPSIPMTProgramElement *a;

	if ((!it) || (!b)) return;

	for (i = 0; i < (int)it->len; i++) {
		a = &it->c[i];

		if (a->elementary_PID == b->elementary_PID) return a;
	}

	if (it->cap == 0) {
		it->c = (MPEGTSPSIPMTProgramElement*)calloc(
			1,
			sizeof(MPEGTSPSIPMTProgramElement));
	} else {
		it->c = (MPEGTSPSIPMTProgramElement*)realloc(
			it->c,
			(it->cap + 1)*sizeof(MPEGTSPSIPMTProgramElement));
	}
	it->cap++;
	it->len++;

	a = &it->c[it->len-1];
	*a = *b;

	return a;
}

static MPEGTSPSIPMTESInfo* mpegts_psi_pmt_es_infos_push_unique(MPEGTSPSIPMTESInfos *it, MPEGTSPSIPMTESInfo *b) {
	int i = 0;
	MPEGTSPSIPMTESInfo *a;

	if ((!it) || (!b)) return;

	for (i = 0; i < (int)it->len; i++) {
		a = &it->c[i];

		if (a->descriptor_tag == b->descriptor_tag) return a;
	}

	if (it->cap == 0) {
		it->c = (MPEGTSPSIPMTESInfo*)calloc(
			1,
			sizeof(MPEGTSPSIPMTESInfo));
	} else {
		it->c = (MPEGTSPSIPMTESInfo*)realloc(
			it->c,
			(it->cap + 1)*sizeof(MPEGTSPSIPMTESInfo));
	}
	it->cap++;
	it->len++;

	a = &it->c[it->len-1];
	*a = *b;

	return a;
}

MPEGTSPSIPMTProgramElement *mpegts_psi_pmt_search_by_es_type(MPEGTSPSIPMT *it, MPEGTSESType q) {
	int i = 0;
	MPEGTSPSIPMTProgramElement *pe = NULL;

	if (!it) return pe;
	if (!it->program_elements.c) return pe;

	for (i = 0; i < it->program_elements.len; i++) {
		pe = &it->program_elements.c[i];
		if (pe->stream_type == q)
			return pe;
	}

	return pe;
}

void mpegts_psi_pmt_print_json(MPEGTSPSIPMT *it) {}

void mpegts_psi_pmt_del(MPEGTSPSIPMT *it) {
	if (!it) return;

	free(it);
}
