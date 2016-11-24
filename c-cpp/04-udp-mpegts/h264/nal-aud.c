#include "h264.h"

int h264_nal_aud_parse(H264NALAUD *it, const uint8_t *data) {
	uint32_t offset = 0;
	it->primary_pic_type = h264_bitstream_get_bits(data, &offset, 3);
}

void h264_nal_aud_print_json(H264NALAUD *it) {}

void h264_nal_aud_print_humanized(H264NALAUD *it) {
	printf("H264 AUD:\n");
	printf("  primary-pic-type: %d\n", it->primary_pic_type);
}

