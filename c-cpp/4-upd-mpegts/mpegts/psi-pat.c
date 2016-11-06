#include "mpegts.h"


MPEGTSPSIPAT *mpegts_psi_pat_new(void) {
	MPEGTSPSIPAT *it = calloc(1, sizeof(MPEGTSPSIPAT));
	return it;
}

void mpegts_psi_pat_parse(MPEGTSPSIPAT *it, uint8_t *data) {
	mpegts_psi_parse(&it->psi, data);
	// psi-size x8
	data += 8;

	if (it->psi.table_id != MPEGTS_TABLE_ID_PROGRAM_ASSOCIATION_SECTION) {
		fprintf(stderr, "[psi-pat @ %p] error parsing PAT table - got bad table-id %02X\n",
			it, it->psi.table_id);
		return;
	}

	it->program_number = (((uint16_t)data[0] & 0xFF) << 8) | ((uint16_t)data[1] & 0xFF);
	it->program_map_PID = (((uint16_t)data[2] & 0x1F) << 8) | ((uint16_t)data[3] & 0xFF);
}

void mpegts_psi_pat_print_json(MPEGTSPSIPAT *it) {
	printf(
		"{\"program-number\": %d"
		", \"program-map-PID\": %d"
		"}\n",
		it->program_number,
		it->program_map_PID
	);
}

void mpegts_psi_pat_del(MPEGTSPSIPAT *it) {
	if (!it) return;

	free(it);
}
