#include "h264.h"
#include "../color.h"


const char* h264_nal_type_string(H264NALType it) {
	switch(it) {
	case H264_NAL_TYPE_UNSPECIFIED: return H264_NAL_TYPE_UNSPECIFIED_STR;
	case H264_NAL_TYPE_DPA: return H264_NAL_TYPE_DPA_STR;
	case H264_NAL_TYPE_DPB: return H264_NAL_TYPE_DPB_STR;
	case H264_NAL_TYPE_DPC: return H264_NAL_TYPE_DPC_STR;
	case H264_NAL_TYPE_IDR: return H264_NAL_TYPE_IDR_STR;
	case H264_NAL_TYPE_SEI: return H264_NAL_TYPE_SEI_STR;
	case H264_NAL_TYPE_SPS: return H264_NAL_TYPE_SPS_STR;
	case H264_NAL_TYPE_PPS: return H264_NAL_TYPE_PPS_STR;
	case H264_NAL_TYPE_AUD: return H264_NAL_TYPE_AUD_STR;
	case H264_NAL_TYPE_EOSEQ: return H264_NAL_TYPE_EOSEQ_STR;
	case H264_NAL_TYPE_EOSTREAM: return H264_NAL_TYPE_EOSTREAM_STR;
	case H264_NAL_TYPE_FILL: return H264_NAL_TYPE_FILL_STR;
	case H264_NAL_TYPE_SPS_EXT: return H264_NAL_TYPE_SPS_EXT_STR;
	case H264_NAL_TYPE_PREFIX: return H264_NAL_TYPE_PREFIX_STR;
	case H264_NAL_TYPE_SSPS: return H264_NAL_TYPE_SSPS_STR;
	case H264_NAL_TYPE_DPS: return H264_NAL_TYPE_DPS_STR;
	case H264_NAL_TYPE_CSOACPWP: return H264_NAL_TYPE_CSOACPWP_STR;
	case H264_NAL_TYPE_CSE: return H264_NAL_TYPE_CSE_STR;
	case H264_NAL_TYPE_CSE3D: return H264_NAL_TYPE_CSE3D_STR;
	}

	return "";
}

// TODO: remove debug
static FILE *f_dump_h264 = NULL;
static int is_dump_h264_opened = 0;
static int nal_size = 0;

// TODO: remove extra args: app_offset, es_offset, es_length
void h264_annexb_parse(H264 *it, const uint8_t *data, uint64_t app_offset, int es_offset, int es_length) {
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
			uint8_t forbidden_zero_bit = data[es_i] & 0x80;
			if (forbidden_zero_bit != 0) continue;

			uint8_t nal_ref_idc = data[es_i] & 0x60;
			uint8_t nal_type = data[es_i] & 0x1F;

			const char *nal_type_name = h264_nal_type_string(nal_type);

			printf("ES 0x%08llX | %d | ", es_offset_current, nal_size);

			if (
				(nal_type == H264_NAL_TYPE_AUD) ||
				(nal_type == H264_NAL_TYPE_SEI) ||
				(nal_type == H264_NAL_TYPE_SLICE) ||
				(nal_type == H264_NAL_TYPE_IDR) ||
				(nal_type == H264_NAL_TYPE_SPS) ||
				(nal_type == H264_NAL_TYPE_PPS)
			) {
				switch (nal_type) {
				case H264_NAL_TYPE_AUD:
					printf(COLOR_BRIGHT_YELLOW "%s" COLOR_RESET "\n", nal_type_name);
					break;
				case H264_NAL_TYPE_SEI:
					printf(COLOR_BRIGHT_BLUE "%s" COLOR_RESET "\n", nal_type_name);
					break;
				case H264_NAL_TYPE_SPS:
					printf(COLOR_BRIGHT_WHITE "%s" COLOR_RESET "\n", nal_type_name);
					break;
				case H264_NAL_TYPE_PPS:
					printf(COLOR_BRIGHT_WHITE "%s" COLOR_RESET "\n", nal_type_name);
					break;
				}

				if (nal_type == H264_NAL_TYPE_SPS) {
					h264_nal_sps_parse(&it->nal_sps, &data[es_i+1]);
					// h264_nal_sps_print_humanized(&it->nal_sps);
					it->got_nal_sps = 1;
				} else if (nal_type == H264_NAL_TYPE_PPS) {
					h264_nal_pps_parse(&it->nal_pps, &data[es_i+1]);
					// h264_nal_pps_print_humanized(&it->nal_pps);
					it->got_nal_pps = 1;
				} else if (nal_type == H264_NAL_TYPE_AUD) {
					h264_nal_aud_parse(&it->nal_aud, &data[es_i+1]);
					// h264_nal_aud_print_humanized(&it->nal_aud);
					it->got_nal_aud = 1;
				} else if ((it->got_nal_sps) &&
				           ((nal_type == H264_NAL_TYPE_SLICE) ||
				            (nal_type == H264_NAL_TYPE_IDR))) {
					h264_nal_slice_idr_parse(&it->nal_slice_idr, &it->nal_sps, &data[es_i+1]);
					h264_nal_slice_idr_print_humanized_one_line(&it->nal_slice_idr);

					nal_size = 0;
				}
			} else {
				// printf("!! ES 0x%08llX %d\n", es_offset_current, nal_type);
			}
		}
	}
}
