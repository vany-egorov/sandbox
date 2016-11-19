#include "h264.h"
#include "../color.h"


const char* h264_nal_type_string(H264NALType it) {
	switch(it) {
	case NAL_TYPE_UNSPECIFIED: return NAL_TYPE_UNSPECIFIED_STR;
	case NAL_TYPE_DPA: return NAL_TYPE_DPA_STR;
	case NAL_TYPE_DPB: return NAL_TYPE_DPB_STR;
	case NAL_TYPE_DPC: return NAL_TYPE_DPC_STR;
	case NAL_TYPE_IDR: return NAL_TYPE_IDR_STR;
	case NAL_TYPE_SEI: return NAL_TYPE_SEI_STR;
	case NAL_TYPE_SPS: return NAL_TYPE_SPS_STR;
	case NAL_TYPE_PPS: return NAL_TYPE_PPS_STR;
	case NAL_TYPE_AUD: return NAL_TYPE_AUD_STR;
	case NAL_TYPE_EOSEQ: return NAL_TYPE_EOSEQ_STR;
	case NAL_TYPE_EOSTREAM: return NAL_TYPE_EOSTREAM_STR;
	case NAL_TYPE_FILL: return NAL_TYPE_FILL_STR;
	case NAL_TYPE_SPS_EXT: return NAL_TYPE_SPS_EXT_STR;
	case NAL_TYPE_PREFIX: return NAL_TYPE_PREFIX_STR;
	case NAL_TYPE_SSPS: return NAL_TYPE_SSPS_STR;
	case NAL_TYPE_DPS: return NAL_TYPE_DPS_STR;
	case NAL_TYPE_CSOACPWP: return NAL_TYPE_CSOACPWP_STR;
	case NAL_TYPE_CSE: return NAL_TYPE_CSE_STR;
	case NAL_TYPE_CSE3D: return NAL_TYPE_CSE3D_STR;
	}

	return "";
}

static inline uint32_t get_bit(const uint8_t * const base, uint32_t offset) {
	return ((*(base + (offset >> 0x3))) >> (0x7 - (offset & 0x7))) & 0x1;
}

static inline uint32_t get_bits(const uint8_t * const base, uint32_t * const offset, uint8_t bits) {
	int i = 0;
	uint32_t value = 0;
	for (i = 0; i < bits; i++) {
		value = (value << 1) | (get_bit(base, (*offset)++) ? 1 : 0);
	}
	return value;
}

// This function implement decoding of exp-Golomb codes of zero range (used in H.264).
static uint32_t decode_u_golomb(const uint8_t * const base, uint32_t * const offset) {
	uint32_t zeros = 0;

	// calculate zero bits. Will be optimized.
	while (0 == get_bit(base, (*offset)++)) zeros++;

	// insert first 1 bit
	uint32_t info = 1 << zeros;

	int32_t i = 0;
	for (i = zeros - 1; i >= 0; i--) {
		info |= get_bit(base, (*offset)++) << i;
	}

	return (info - 1);
}

// TODO: remove debug
static FILE *f_dump_h264 = NULL;
static int is_dump_h264_opened = 0;
static int nal_size = 0;

// TODO: remove extra args: app_offset, es_offset, es_length
void h264_annexb_parse(H264AnnexBNALU *it, const uint8_t *data, uint64_t app_offset, int es_offset, int es_length) {
	int es_i = 0;
	uint64_t es_offset_current = 0;
	uint32_t es_start_code = 0;
	int got_es_start_code = 0;

	if (1) {
		if (!is_dump_h264_opened) {
			f_dump_h264 = fopen("./tmp/out.h264", "wb");
			is_dump_h264_opened = 1;
		}
		nal_size += es_length;
		fwrite(data, es_length, 1, f_dump_h264);
	}

	for (es_i = 0; es_i < es_length; es_i++) {
		es_start_code = 0;
		got_es_start_code = 0;

		es_offset_current = app_offset + es_offset + es_i;

		es_start_code = (
			(uint32_t)data[es_i]   << 24 |
			(uint32_t)data[es_i+1] << 16 |
			(uint32_t)data[es_i+2] << 8  |
			(uint32_t)data[es_i+3]
		);
		if (es_start_code == H264_ANNEXB_START_CODE_SHORT) {
			es_i += 4;
			got_es_start_code = 1;
		} else {
			es_start_code = (
				0                      << 24 |
				(uint32_t)data[es_i]   << 16 |
				(uint32_t)data[es_i+1] << 8  |
				(uint32_t)data[es_i+2]
			);
			if (es_start_code == H264_ANNEXB_START_CODE_LONG) {
				es_i += 3;
				got_es_start_code = 1;
			}
		}

		if (got_es_start_code) {
			printf("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
				data[es_i+1],
				data[es_i+2],
				data[es_i+3],
				data[es_i+4],
				data[es_i+5],
				data[es_i+6],
				data[es_i+7],
				data[es_i+8],
				data[es_i+9],
				data[es_i+10],
				data[es_i+11]
			);

			uint8_t forbidden_zero_bit = data[es_i] & 0x80;
			if (forbidden_zero_bit != 0) continue;

			uint8_t nal_ref_idc = data[es_i] & 0x60;
			uint8_t nal_type = data[es_i] & 0x1F;

			const char *nal_type_name = h264_nal_type_string(nal_type);

			if (
				(nal_type == NAL_TYPE_AUD) ||
				(nal_type == NAL_TYPE_SEI) ||
				(nal_type == NAL_TYPE_SLICE) ||
				(nal_type == NAL_TYPE_IDR) ||
				(nal_type == NAL_TYPE_SPS) ||
				(nal_type == NAL_TYPE_PPS)
			) {
				switch (nal_type) {
				case NAL_TYPE_AUD:
					printf("ES 0x%08llX | " COLOR_BRIGHT_YELLOW "%s" COLOR_RESET "\n",
						es_offset_current, nal_type_name);
					break;
				case NAL_TYPE_SEI:
					printf("ES 0x%08llX | " COLOR_BRIGHT_BLUE "%s" COLOR_RESET "\n",
						es_offset_current, nal_type_name);
					break;
				case NAL_TYPE_SPS:
					printf("ES 0x%08llX | " COLOR_BRIGHT_WHITE "%s" COLOR_RESET "\n",
						es_offset_current, nal_type_name);
					break;
				case NAL_TYPE_PPS:
					printf("ES 0x%08llX | " COLOR_BRIGHT_WHITE "%s" COLOR_RESET "\n",
						es_offset_current, nal_type_name);
					break;
				}

				// slice hader
				if ((nal_type == NAL_TYPE_SLICE) || (nal_type == NAL_TYPE_IDR)) {
					uint32_t offset = 0;
					// printf(">>> 1 - %d\n", offset);
					uint32_t first_mb_in_slice = decode_u_golomb(&data[es_i+1], &offset);
					// printf(">>> 2 - %d\n", offset);
					uint32_t slice_type = decode_u_golomb(&data[es_i+1], &offset);
					// printf(">>> 3 - %d\n", offset);
					uint32_t pic_parameter_set_id = decode_u_golomb(&data[es_i+1], &offset);
					// printf(">>> 4 - %d\n", offset);
					uint32_t frame_num = get_bits(&data[es_i+1], &offset, 9); // from SPS
					// printf(">>> 5 - %d\n", offset);
					uint32_t pic_order_cnt_lsb = get_bits(&data[es_i+1], &offset, 10); // from SPS
					// printf(">>> 6 - %d\n", offset);

					// 0 => P-slice. Consists of P-macroblocks
					//      (each macro block is predicted using
					//      one reference frame) and / or I-macroblocks.
					// 1 => B-slice. Consists of B-macroblocks (each
					//      macroblock is predicted using one or two
					//      reference frames) and / or I-macroblocks.
					// 2 => I-slice. Contains only I-macroblocks.
					//      Each macroblock is predicted from previously
					//      coded blocks of the same slice.
					// 3 => SP-slice. Consists of P and / or I-macroblocks
					//      and lets you switch between encoded streams.
					// 4 => SI-slice. It consists of a special type
					//      of SI-macroblocks and lets you switch between
					//      encoded streams.
					// 5 => P-slice.
					// 6 => B-slice.
					// 7 => I-slice.
					// 8 => SP-slice.
					// 9 => SI-slice.
					switch (slice_type) {
					case 0:
					case 5:
						printf("ES 0x%08llX | %d | " COLOR_BRIGHT_CYAN "H264 P slice #%d { slice-type: %d, frame-num: %d, pic-order-cnt-lsb: %d }" COLOR_RESET "\n",
							es_offset_current, nal_size, 0, slice_type, frame_num, pic_order_cnt_lsb);
						break;
					case 1:
					case 6:
						printf("ES 0x%08llX | %d | " COLOR_BRIGHT_GREEN "H264 B slice #%d { slice-type: %d, frame-num: %d, pic-order-cnt-lsb: %d }" COLOR_RESET "\n",
							es_offset_current, nal_size, 0, slice_type, frame_num, pic_order_cnt_lsb);
						break;
					case 2:
					case 7:
						printf("ES 0x%08llX | %d | " COLOR_BRIGHT_RED "H264 I slice #%d { slice-type: %d, frame-num: %d, pic-order-cnt-lsb: %d }" COLOR_RESET "\n",
							es_offset_current, nal_size, 0, slice_type, frame_num, pic_order_cnt_lsb);
						break;
					default:
						printf("ES 0x%08llX | " COLOR_BRIGHT_RED "H264 slice #%d { slice-type: %d }" COLOR_RESET "\n",
							es_offset_current, frame_num, slice_type);
						break;
					}

					nal_size = 0;
				}
			} else {
				// printf("!! ES 0x%08llX %d\n", es_offset_current, nal_type);
			}
		}
	}
}
