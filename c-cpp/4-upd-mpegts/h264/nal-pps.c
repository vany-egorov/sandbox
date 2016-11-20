#include "h264.h"


int h264_nal_pps_parse(H264NALPPS *it, const uint8_t *data) {
	uint32_t offset = 0;

	it->pic_parameter_set_id = h264_bitstream_decode_u_golomb(data, &offset);
	it->seq_parameter_set_id = h264_bitstream_decode_u_golomb(data, &offset);
	it->entropy_coding_mode_flag = h264_bitstream_get_bits(data, &offset, 1);
	it->bottom_field_pic_order_in_frame_present_flag = h264_bitstream_get_bits(data, &offset, 1);
	it->num_slice_groups_minus1 = h264_bitstream_decode_u_golomb(data, &offset);
}

void h264_nal_pps_print_json(H264NALPPS *it) {}

void h264_nal_pps_print_humanized(H264NALPPS *it) {
	printf("H264 PPS:\n");
	printf("  pic-parameter-set-id: %d\n", it->pic_parameter_set_id);
	printf("  seq-parameter-set-id: %d\n", it->seq_parameter_set_id);
	printf("  entropy-coding-mode-flag: %d\n", it->entropy_coding_mode_flag);
	printf("  bottom-field-pic-order-in-frame-present-flag: %d\n", it->bottom_field_pic_order_in_frame_present_flag);
	printf("  num-slice-groups-minus1: %d\n", it->num_slice_groups_minus1);
}
