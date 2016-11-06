#include "mpegts.h"


static void                    mpegts_psi_sdt_service_parse_header(MPEGTSPSISDTService *it, uint8_t *data);
static void                    mpegts_psi_sdt_service_parse_descriptors(MPEGTSPSISDTService *it, uint8_t *data);
static MPEGTSPSISDTService*    mpegts_psi_sdt_services_push_unique(MPEGTSPSISDTServices *it, MPEGTSPSISDTService *b);
static void                    mpegts_psi_sdt_descriptor_parse(MPEGTSPSISDTDescriptor *it, uint8_t *data);
static MPEGTSPSISDTDescriptor* mpegts_psi_sdt_descriptors_push_unique(MPEGTSPSISDTDescriptors *it, MPEGTSPSISDTDescriptor *b);


MPEGTSPSISDT *mpegts_psi_sdt_new(void) {
	MPEGTSPSISDT *it = calloc(1, sizeof(MPEGTSPSISDT));
	return it;
}

void mpegts_psi_sdt_parse(MPEGTSPSISDT *it, uint8_t *data) {
	int pos = 0, // program-element => parser position current
	    end = 0; // program-element => finish

	mpegts_psi_parse(&it->psi, data);

	if (it->psi.table_id != MPEGTS_TABLE_ID_SERVICE_DESCRIPTION_SECTION_ACTUAL_TRANSPORT_STREAM) {
		fprintf(stderr, "[psi-sdt @ %p] error parsing SDT table - got bad table-id %02X\n",
			it, it->psi.table_id);
		return;
	}

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

	it->original_network_id = (
		(((uint16_t)data[pos++] & 0xFF) << 8) |
		 ((uint16_t)data[pos++] & 0xFF)
	);
	it->reserved_future_use = (uint8_t)data[pos++];

	while (pos < end) {
		MPEGTSPSISDTService b = { 0 },
		                   *a = NULL;
		mpegts_psi_sdt_service_parse_header(&b, &data[pos]);
		a = mpegts_psi_sdt_services_push_unique(&it->services, &b);
		mpegts_psi_sdt_service_parse_descriptors(a, &data[pos+5]);

		// service_id                   x2;
		// reserved_future_use,
		//   EIT_schedule_flag,
		//   EIT_present_following_flag x1;
		// running_status
		//   free_CA_mode
		//   descriptors_loop_length    x2;
		//
		// -----                        x5;
		pos += 5;
		pos += (int)a->descriptors_loop_length;
		printf("===> %d | %d (%s)\n",
			a->descriptors_loop_length,
			a->running_status,
			mpegts_psi_sdt_service_running_status_string(a->running_status)
		);
	}
	printf("===> %d\n", it->original_network_id);
}

static void mpegts_psi_sdt_service_parse_header(MPEGTSPSISDTService *it, uint8_t *data) {
	it->service_id = (
		(((uint16_t)data[0] & 0xFF) << 8) |
		((uint16_t)data[1] & 0xFF)
	);

	it->reserved_future_use = ((uint8_t)data[2] & 0xFC) >> 2;
	it->EIT_schedule_flag = !!( (uint8_t)data[2] & 0x02 );
	it->EIT_present_following_flag = !!( (uint8_t)data[2] & 0x01 );

	it->running_status = ((uint8_t)data[3] & 0xE0) >> 5;
	it->free_CA_mode = !!( (uint8_t)data[3] & 0x10 );
	it->descriptors_loop_length = (
		(((uint16_t)data[3] & 0x0F) << 8) |
		((uint16_t)data[4] & 0xFF)
	);
}

static void mpegts_psi_sdt_service_parse_descriptors(MPEGTSPSISDTService *it, uint8_t *data) {
	int pos = 0, // Descriptor => parser position current
	    end = 0; // Descriptor => finish

	if (it->descriptors_loop_length) {
		pos = 0;
		end = (int)it->descriptors_loop_length;

		while (pos < end) {
			MPEGTSPSISDTDescriptor descriptor = { 0 };
			mpegts_psi_sdt_descriptor_parse(&descriptor, &data[pos]);
			mpegts_psi_sdt_descriptors_push_unique(&it->descriptors, &descriptor);

			// descriptor_tag    x1;
			// descriptor_length x1;
			pos += 2;
			pos += (int)descriptor.descriptor_length;
		}
	}
}

void mpegts_psi_sdt_descriptor_parse(MPEGTSPSISDTDescriptor *it, uint8_t *data) {
	int pos = 0;

	it->descriptor_tag = (MPEGTSPSIPEDTag)data[pos++];
	it->descriptor_length = (uint8_t)data[pos++];

	if (it->descriptor_length) {
		switch (it->descriptor_tag) {
		case MPEGTS_PSI_PED_TAG_SERVICE_DESCRIPTOR:
			it->descriptor_data.service.service_type = (uint8_t)data[pos++];

			it->descriptor_data.service.service_provider_name_length = (uint8_t)data[pos++];
			memcpy(
				it->descriptor_data.service.service_provider_name,
				&data[pos],
				(sizeof(it->descriptor_data.service.service_provider_name)-1) <= (size_t)it->descriptor_data.service.service_provider_name_length
					? (sizeof(it->descriptor_data.undefined.data)-1)
					: (size_t)it->descriptor_data.service.service_provider_name_length
			);
			pos += (int)it->descriptor_data.service.service_provider_name_length;

			it->descriptor_data.service.service_name_length = (uint8_t)data[pos++];
			memcpy(
				it->descriptor_data.service.service_name,
				&data[pos],
				(sizeof(it->descriptor_data.service.service_name)-1) <= (size_t)it->descriptor_data.service.service_name_length
					? (sizeof(it->descriptor_data.undefined.data)-1)
					: (size_t)it->descriptor_data.service.service_name_length
			);

			printf("~~~ \t service-type: %d;\n", it->descriptor_data.service.service_type);
			printf("~~~ \t service-provider-name-length: %d;\n", it->descriptor_data.service.service_provider_name_length);
			printf("~~~ \t service-provider-name: \"%s\";\n", it->descriptor_data.service.service_provider_name);
			printf("~~~ \t service-name-length: %d;\n", it->descriptor_data.service.service_name_length);
			printf("~~~ \t service-name: \"%s\";\n", it->descriptor_data.service.service_name);
			break;

		default:
			memcpy(
				it->descriptor_data.undefined.data, // dst
				&data[2],                           // src
				(sizeof(it->descriptor_data.undefined.data)-1) <= (size_t)it->descriptor_length
					? (sizeof(it->descriptor_data.undefined.data)-1)
					: (size_t)it->descriptor_length
			);

			int i;
			printf("~~~ \t ");
			for (i=0; i < (int)it->descriptor_length; i++) {
				printf("%c", it->descriptor_data.undefined.data[i]);
			}
			printf("\n");
		}
	}
	// printf("~~~ \t (%X \"%s\") %d\n", it->descriptor_tag, mpegts_psi_ped_tag_string(it->descriptor_tag), it->descriptor_length);
	// if (it->descriptor_length)
	// 	memcpy(
	// 		it->descriptor_data, // dst
	// 		&data[2],            // src
	// 		sizeof(it->descriptor_data) <= (size_t)it->descriptor_length
	// 			? (size_t)it->descriptor_length
	// 			: sizeof(it->descriptor_data)
	// 	);

	// if (it->descriptor_length) {
	// 	int i;
	// 	printf("~~~ \t ");
	// 	for (i=0; i < (int)it->descriptor_length; i++) {
	// 		printf("%c", it->descriptor_data[i]);
	// 	}
	// 	printf("\n");
	// }
}

const char *mpegts_psi_sdt_service_running_status_string(MPEGTSPSISDTServiceRunningStatus it) {
	switch(it) {
	case MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_UNDEFINED:               return MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_UNDEFINED_STR;
	case MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_NOT_RUNNING:             return MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_NOT_RUNNING_STR;
	case MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_STARTS_IN_A_FEW_SECONDS: return MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_STARTS_IN_A_FEW_SECONDS_STR;
	case MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_PAUSING:                 return MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_PAUSING_STR;
	case MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_RUNNING:                 return MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_RUNNING_STR;
	case MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_RESERVED_05:             return MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_RESERVED_05_STR;
	case MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_RESERVED_06:             return MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_RESERVED_06_STR;
	case MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_RESERVED_07:             return MPEGTS_PSI_SDT_SERVICE_RUNNING_STATUS_RESERVED_07_STR;
	}

	return "unknown PSI->SDT->Service running status";
}

static MPEGTSPSISDTService* mpegts_psi_sdt_services_push_unique(MPEGTSPSISDTServices *it, MPEGTSPSISDTService *b) {
	int i = 0;
	MPEGTSPSISDTService *a;

	if ((!it) || (!b)) return;

	for (i = 0; i < (int)it->len; i++) {
		a = &it->c[i];

		if (a->service_id == b->service_id) return a;
	}

	if (it->cap == 0) {
		it->c = (MPEGTSPSISDTService*)calloc(
			1,
			sizeof(MPEGTSPSISDTService));
	} else {
		it->c = (MPEGTSPSISDTService*)realloc(
			it->c,
			(it->cap + 1)*sizeof(MPEGTSPSISDTService));
	}
	it->cap++;
	it->len++;

	a = &it->c[it->len-1];
	*a = *b;

	return a;
}

static MPEGTSPSISDTDescriptor* mpegts_psi_sdt_descriptors_push_unique(MPEGTSPSISDTDescriptors *it, MPEGTSPSISDTDescriptor *b) {
	int i = 0;
	MPEGTSPSISDTDescriptor *a;

	if ((!it) || (!b)) return;

	for (i = 0; i < (int)it->len; i++) {
		a = &it->c[i];

		if (a->descriptor_tag == b->descriptor_tag) return a;
	}

	if (it->cap == 0) {
		it->c = (MPEGTSPSISDTDescriptor*)calloc(
			1,
			sizeof(MPEGTSPSISDTDescriptor));
	} else {
		it->c = (MPEGTSPSISDTDescriptor*)realloc(
			it->c,
			(it->cap + 1)*sizeof(MPEGTSPSISDTDescriptor));
	}
	it->cap++;
	it->len++;

	a = &it->c[it->len-1];
	*a = *b;

	return a;
}

void mpegts_psi_sdt_del(MPEGTSPSISDT *it) {
	if (!it) return;

	free(it);
}
