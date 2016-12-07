#include "va.h"


void* parser_worker_read_do(void *args) {
	VAParserWorkerRead *it = NULL;
	uint8_t buf[MPEGTS_PACKET_COUNT*MPEGTS_PACKET_SIZE] = { 0 };
	size_t copied = 0;

	it = (VAParserWorkerRead*)args;

	for(;;)
		io_copy(it->reader, it->writer, buf, sizeof(buf), &copied);

	return NULL;
}
