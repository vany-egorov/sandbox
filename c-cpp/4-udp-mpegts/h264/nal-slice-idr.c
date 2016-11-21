#include "h264.h"
#include "../color.h"


int h264_nal_slice_idr_parse(H264NALSliceIDR *it, H264NALSPS *sps, const uint8_t *data) {
	if (!sps) return;

	uint32_t offset = 0;
	it->first_mb_in_slice = h264_bitstream_decode_u_golomb(data, &offset);
	it->slice_type = h264_bitstream_decode_u_golomb(data, &offset);
	it->pic_parameter_set_id = h264_bitstream_decode_u_golomb(data, &offset);
	it->frame_num = h264_bitstream_get_bits(data, &offset, sps->log2_max_frame_num_minus4+4);
	it->pic_order_cnt_lsb = h264_bitstream_get_bits(data, &offset, sps->log2_max_pic_order_cnt_lsb_minus4+4);
}

void h264_nal_slice_idr_print_humanized_one_line(H264NALSliceIDR *it) {
	switch (it->slice_type) {
	case H264_NAL_SLICE_TYPE_P:
	case H264_NAL_SLICE_TYPE_P2:
		printf(COLOR_BRIGHT_CYAN "H264 P slice #%d { frame-num: %d, pic-order-cnt-lsb: %d }" COLOR_RESET "\n",
			0, it->slice_type, it->frame_num, it->pic_order_cnt_lsb);
		break;
	case H264_NAL_SLICE_TYPE_B:
	case H264_NAL_SLICE_TYPE_B2:
		printf(COLOR_BRIGHT_GREEN "H264 B slice #%d { frame-num: %d, pic-order-cnt-lsb: %d }" COLOR_RESET "\n",
			0, it->slice_type, it->frame_num, it->pic_order_cnt_lsb);
		break;
	case H264_NAL_SLICE_TYPE_I:
	case H264_NAL_SLICE_TYPE_I2:
		printf(COLOR_BRIGHT_RED "H264 I slice #%d { frame-num: %d, pic-order-cnt-lsb: %d }" COLOR_RESET "\n",
			0, it->slice_type, it->frame_num, it->pic_order_cnt_lsb);
		break;
	default:
		printf(COLOR_BRIGHT_RED "H264 slice #%d { slice-type: %d, frame-num: %d, pic-order-cnt-lsb: %d }" COLOR_RESET "\n",
			0, it->slice_type, it->frame_num, it->pic_order_cnt_lsb);
		break;
	}
}
