#include "mpegts.h"


MPEGTSHeader *mpegts_header_new(void) {
	MPEGTSHeader *it = calloc(1, sizeof(MPEGTSHeader));
	return it;
}

void mpegts_header_parse(MPEGTSHeader *it, uint8_t *data) {
	uint8_t transcport_error_indicator = 0;
	uint8_t payload_unit_start_indicator = 0;
	uint8_t transcport_priority = 0;
	uint16_t PID = 0;
	uint8_t transport_scrambling_control = 0;
	uint8_t adaption_field_control = 0;
	uint8_t contains_payload = 0;
	uint8_t continuity_counter = 0;

	transcport_error_indicator = !!( (uint8_t)data[0] & 0x80 );
	payload_unit_start_indicator = !!( (uint8_t)data[0] & 0x40 );
	transcport_priority = !!( (uint8_t)data[0] & 0x20 );
	PID = ( (uint16_t)data[0] << 8 | (uint16_t)data[1] ) & 0x1fff;
	transport_scrambling_control = ((uint8_t)data[2] & 0xc0) >> 6;
	adaption_field_control = !!( (uint8_t)data[2] & 0x20 );
	contains_payload = !!( (uint8_t)data[2] & 0x10 );
	continuity_counter = (uint8_t)data[2] & 0xf;

	it->transcport_error_indicator = transcport_error_indicator;
	it->payload_unit_start_indicator = payload_unit_start_indicator;
	it->transcport_priority = transcport_priority;
	it->PID = PID;
	it->transport_scrambling_control = transport_scrambling_control;
	it->adaption_field_control = adaption_field_control;
	it->contains_payload = contains_payload;
	it->continuity_counter = continuity_counter;
}

void mpegts_header_print_json(MPEGTSHeader *it) {
	printf(
		"{\"TEI\": %d"
		", \"PUSI\": %d"
		", \"TP\": %d"
		", \"PID\": 0x%04x"
		", \"TSC\": 0x%02x"
		", \"adaption\": %d"
		", \"payload\": %d"
		", \"continuity-counter\": %d"
		"}\n",
		it->transcport_error_indicator,
		it->payload_unit_start_indicator,
		it->transcport_priority,
		it->PID,
		it->transport_scrambling_control,
		it->adaption_field_control,
		it->contains_payload,
		it->continuity_counter
	);
}

void mpegts_header_del(MPEGTSHeader* it) {
	if (!it) return;

	free(it);
}
