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
	case MPEGTS_PSI_PED_TAG_SERVICE_DESCRIPTOR:            return MPEGTS_PSI_PED_TAG_SERVICE_DESCRIPTOR_STR;
	}

	return "unknown Program Element Descriptor tag";
}

void mpegts_psi_pmt_parse(MPEGTSPSIPMT *it, uint8_t *data) {
	int pos = 0, // program-element => parser position current
	    end = 0; // program-element => finish

	mpegts_psi_parse(&it->psi, data);

	if (it->psi.table_id != MPEGTS_TABLE_ID_PROGRAM_MAP_SECTION) {
		fprintf(stderr, "[psi-pmt @ %p] error parsing PMT table - got bad table-id %02X\n",
			it, it->psi.table_id);
		return;
	}

	// psi-size x8
	data += 8;

	end = (int)it->psi.section_length;
	// transport-stream-id   x2;
	// version-number        x1;
	// curent-next-indicator
	// section-number        x1;
	// last-section-number   x1;
	// CRC32                 x4;
	//
	// -----                 x9;
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

	if (it->descriptor_length) {
		switch (it->descriptor_tag) {
		case MPEGTS_PSI_PED_TAG_ISO_639:
			it->descriptor_data.language.code[0] = data[2];
			it->descriptor_data.language.code[1] = data[3];
			it->descriptor_data.language.code[2] = data[4];
			it->descriptor_data.language.code[3] = '\0';
			it->descriptor_data.language.audio_type = (uint8_t)data[5];
			break;

		default:
			memcpy(
				it->descriptor_data.undefined.data, // dst
				&data[2],                           // src
				(sizeof(it->descriptor_data.undefined.data)-1) <= (size_t)it->descriptor_length
					? (sizeof(it->descriptor_data.undefined.data)-1)
					: (size_t)it->descriptor_length
			);
		}
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

void mpegts_psi_pmt_print_humanized(MPEGTSPSIPMT *it) {
	int i = 0,
	    j = 0;
	MPEGTSPSIPMTProgramElement *pe;
	MPEGTSPSIPMTESInfo *ei;
	MPEGTSPSIDescriptorDataLanguage *dlanguage = NULL;
	MPEGTSPSIDescriptorDataUndefined *dund = NULL;

	printf("PMT:\n");
	printf("  psi-transport-stream-id: %d (0x%02X)\n", it->psi.transport_stream_id, it->psi.transport_stream_id);
	printf("  psi-CRC32: 0x%02X\n", it->psi.CRC32);
	printf("  PCR-PID: %d (0x%02X)\n", it->PCR_PID, it->PCR_PID);

	if (it->program_elements.c) {
		printf("  program-elements:\n");
		for (i = 0; i < it->program_elements.len; i++) {
			pe = &it->program_elements.c[i];

			printf("    - stream-type: %d (0x%02X) / \"%s\"\n", pe->stream_type, pe->stream_type, mpegts_es_type_string(pe->stream_type));
			printf("      PID: %d (0x%02X)\n", pe->elementary_PID, pe->elementary_PID);

			if (pe->es_infos.c) {
				printf("      ES-info:\n");
				for (j = 0; j < pe->es_infos.len; j++) {
					ei = &pe->es_infos.c[j];

					printf("        - descriptor-tag: %d (0x%02X) / \"%s\"\n", ei->descriptor_tag, ei->descriptor_tag, mpegts_psi_ped_tag_string(ei->descriptor_tag));
					printf("          descriptor-data:\n");
					switch (ei->descriptor_tag) {
						case MPEGTS_PSI_PED_TAG_ISO_639: {
							dlanguage = &ei->descriptor_data.language;
							printf("            code: \"%s\"\n", dlanguage->code);
							printf("            audio-type: %d (0x%02X)\n", dlanguage->audio_type, dlanguage->audio_type);
							break;
						}
						default: {
							dund = &ei->descriptor_data.undefined;
							printf("            service-type: \"undefined\"\n");
							printf("            data: \"%s\"\n", dund->data);
						}
					}

				}
			}

		}
	}
}

void mpegts_psi_pmt_del(MPEGTSPSIPMT *it) {
	if (!it) return;

	free(it);
}
