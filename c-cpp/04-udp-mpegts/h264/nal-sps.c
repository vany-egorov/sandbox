#include "h264.h"


static int got_profile(H264NALSPS *it) {
	if(it->profile_idc == 100 || it->profile_idc == 110 || it->profile_idc == 122 ||
	   it->profile_idc == 244 || it->profile_idc ==  44 || it->profile_idc == 83 ||
	   it->profile_idc == 86 || it->profile_idc == 118 || it->profile_idc == 128 ) {
		return 1;
	}
	return 0;
}

int h264_nal_sps_parse(H264NALSPS *it, const uint8_t *data) {
	uint32_t offset = 0;

	it->profile_idc = h264_bitstream_get_bits(data, &offset, 8);
	it->constraint_set0_flag = h264_bitstream_get_bits(data, &offset, 1);
	it->constraint_set1_flag = h264_bitstream_get_bits(data, &offset, 1);
	it->constraint_set2_flag = h264_bitstream_get_bits(data, &offset, 1);
	it->constraint_set3_flag = h264_bitstream_get_bits(data, &offset, 1);
	it->constraint_set4_flag = h264_bitstream_get_bits(data, &offset, 1);
	it->constraint_set5_flag = h264_bitstream_get_bits(data, &offset, 1);
	it->reserved_zero_2bits = h264_bitstream_get_bits(data, &offset, 2);
	it->level_idc = h264_bitstream_get_bits(data, &offset, 8);
	it->seq_parameter_set_id = h264_bitstream_decode_u_golomb(data, &offset);

	if (got_profile(it)) {
		it->chroma_format_idc = h264_bitstream_decode_u_golomb(data, &offset);
		if (it->chroma_format_idc == 3)
			it->separate_colour_plane_flag = h264_bitstream_get_bits(data, &offset, 1);
		it->bit_depth_luma_minus8 = h264_bitstream_decode_u_golomb(data, &offset);
		it->bit_depth_chroma_minus8 = h264_bitstream_decode_u_golomb(data, &offset);
		it->qpprime_y_zero_transform_bypass_flag = h264_bitstream_get_bits(data, &offset, 1);
		it->seq_scaling_matrix_present_flag = h264_bitstream_get_bits(data, &offset, 1);
		if (it->seq_scaling_matrix_present_flag) {
			// ...
		}
	}

	it->log2_max_frame_num_minus4 = h264_bitstream_decode_u_golomb(data, &offset);
	it->pic_order_cnt_type = h264_bitstream_decode_u_golomb(data, &offset);
	switch (it->pic_order_cnt_type) {
	case 0:
		it->log2_max_pic_order_cnt_lsb_minus4 = h264_bitstream_decode_u_golomb(data, &offset);
		break;
	case 1:
		it->delta_pic_order_always_zero_flag = h264_bitstream_get_bits(data, &offset, 1);
		it->offset_for_non_ref_pic = h264_bitstream_decode_u_golomb(data, &offset);
		it->offset_for_top_to_bottom_field = h264_bitstream_decode_u_golomb(data, &offset);
		it->num_ref_frames_in_pic_order_cnt_cycle = h264_bitstream_decode_u_golomb(data, &offset);
	}
}

void h264_nal_sps_print_json(H264NALSPS *it) {

}

void h264_nal_sps_print_humanized(H264NALSPS *it) {
	printf("H264 SPS:\n");
	printf("  profile-idc: %d\n", it->profile_idc);
	printf("  constraint-set0-flag: %d\n", it->constraint_set0_flag);
	printf("  constraint-set1-flag: %d\n", it->constraint_set1_flag);
	printf("  constraint-set2-flag: %d\n", it->constraint_set2_flag);
	printf("  constraint-set3-flag: %d\n", it->constraint_set3_flag);
	printf("  constraint-set4-flag: %d\n", it->constraint_set4_flag);
	printf("  constraint-set5-flag: %d\n", it->constraint_set5_flag);
	printf("  reserved-zero-2bits: %d\n", it->reserved_zero_2bits);
	printf("  level-idc: %d\n", it->level_idc);
	printf("  seq-parameter-set-id: %d\n", it->seq_parameter_set_id);
	if (got_profile(it)) {
		printf("  chroma-format-idc: %d\n", it->chroma_format_idc);
		if (it->chroma_format_idc == 3)
			printf("  separate-colour-plane-flag: %d\n", it->separate_colour_plane_flag);
		printf("  bit-depth-luma-minus8: %d\n", it->bit_depth_luma_minus8);
		printf("  bit-depth-chroma-minus8: %d\n", it->bit_depth_chroma_minus8);
		printf("  qpprime-y-zero-transform-bypass-flag: %d\n", it->qpprime_y_zero_transform_bypass_flag);
		printf("  seq-scaling-matrix-present-flag: %d\n", it->seq_scaling_matrix_present_flag);
		if (it->seq_scaling_matrix_present_flag) {
			// ...
		}
	}
	printf("  log2-max-frame-num-minus4: %d\n", it->log2_max_frame_num_minus4);
	printf("  pic-order-cnt-type: %d\n", it->pic_order_cnt_type);
	switch (it->pic_order_cnt_type) {
	case 0:
		printf("    log2-max-pic-order-cnt-lsb-minus4: %d\n", it->log2_max_pic_order_cnt_lsb_minus4);
		break;
	case 1:
		printf("    delta-pic-order-always-zero-flag: %d\n", it->delta_pic_order_always_zero_flag);
		printf("    offset-for-non-ref-pic: %d\n", it->offset_for_non_ref_pic);
		printf("    offset-for-top-to-bottom-field: %d\n", it->offset_for_top_to_bottom_field);
		printf("    num-ref-frames-in-pic-order-cnt-cycle: %d\n", it->num_ref_frames_in_pic_order_cnt_cycle);
	}
}
