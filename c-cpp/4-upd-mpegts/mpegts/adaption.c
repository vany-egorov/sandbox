#include "mpegts.h"


void mpegts_adaption_parse(MPEGTSAdaption *it, uint8_t *data) {
	uint8_t adaptation_field_length	= (uint8_t)data[0];
	uint8_t discontinuity_indicator = !!( (uint8_t)data[1] & 0x80 );
	uint8_t random_access_indicator = !!( (uint8_t)data[1] & 0x40 );
	uint8_t elementary_stream_priority_indicator = !!( (uint8_t)data[1] & 0x20 );
	uint8_t PCR_flag = !!( (uint8_t)data[1] & 0x10 );
	uint8_t OPCR_flag = !!( (uint8_t)data[1] & 0x08 );
	uint8_t splicing_point_flag = !!( (uint8_t)data[1] & 0x04 );
	uint8_t transport_private_data_flag = !!( (uint8_t)data[1] & 0x02 );
	uint8_t adaptation_field_extension_flag = !!( (uint8_t)data[1] & 0x01 );

	it->adaptation_field_length = adaptation_field_length;
	it->discontinuity_indicator = discontinuity_indicator;
	it->random_access_indicator = random_access_indicator;
	it->elementary_stream_priority_indicator = elementary_stream_priority_indicator;
	it->PCR_flag = PCR_flag;
	it->OPCR_flag = OPCR_flag;
	it->splicing_point_flag = splicing_point_flag;
	it->transport_private_data_flag = transport_private_data_flag;
	it->adaptation_field_extension_flag = adaptation_field_extension_flag;

	if (it->PCR_flag)
		mpegts_pcr_parse(&it->PCR, &data[2]);
}

void mpegts_adaption_print_json(MPEGTSAdaption *it) {
	printf(
		"{\"adaptation-field-length\": %d"
		", \"discontinuity-indicator\": %d"
		", \"random-access-indicator\": %d"
		", \"elementary-stream-priority-indicator\": %d"
		", \"PCR-flag\": %d"
		", \"OPCR-flag\": %d"
		", \"splicing-point-flag\": %d"
		", \"transport-private-data-flag\": %d"
		", \"adaptation-field-extension-flag\": %d"
		"}\n",
		it->adaptation_field_length,
		it->discontinuity_indicator,
		it->random_access_indicator,
		it->elementary_stream_priority_indicator,
		it->PCR_flag,
		it->OPCR_flag,
		it->splicing_point_flag,
		it->transport_private_data_flag,
		it->adaptation_field_extension_flag
	);
}

void mpegts_pcr_parse(MPEGTSPCR *it, uint8_t *data) {
	uint64_t base = 0;
	uint16_t ext = 0;

	base = ((
		((uint64_t)data[0] & 0xFF) << 32 |
		((uint64_t)data[1] & 0xFF) << 24 |
		((uint64_t)data[2] & 0xFF) << 16 |
		((uint64_t)data[3] & 0xFF) << 8  |
		((uint64_t)data[4] & 0x80)
	) >> 7);

	ext = (
		((uint16_t)data[4] & 0x01) << 8 |
		((uint16_t)data[5] & 0xFF)
	);

	it->base = base;
	it->ext = ext;
}

void mpegts_pcr_print_json(MPEGTSPCR *it) {
	printf(
		"{\"base\": %" PRIu64 ""
		",\"ext\": %d"
		",\"PCR\": \"0:0:0:XXX (%d)\""
		"}\n",
		it->base,
		it->ext,
		it->base * 300 // 90kHZ => 27MHz
	);
}
