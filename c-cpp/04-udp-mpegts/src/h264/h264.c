#include "h264.h"
#include "../common/color.h"


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
// static size_t nal_size = 0;

int h264_annexb_parse(H264 *it, const uint8_t *data, const size_t datasz, const uint64_t goffset, H264AnnexBParseResult *result) {
	int ret = 0,
	    i = 0,
	    offset = 0,
	    nal_index = 0;
	uint32_t es_start_code = 0;
	uint8_t forbidden_zero_bit = 0,
	        nal_ref_idc = 0;
	H264NALType nal_type = 0;
	int got_es_start_code = 0;

	// TODO: remove debug
	if (0) {
		if (!is_dump_h264_opened) {
			f_dump_h264 = fopen("../tmp/out.h264", "wb");
			is_dump_h264_opened = 1;
		}
		fwrite(data, datasz, 1, f_dump_h264);
	}

	for (i = 0; i < datasz; i++) {
		offset = i;
		es_start_code = 0;
		got_es_start_code = 0;

		es_start_code = (
			(uint32_t)data[i]   << 24 |
			(uint32_t)data[i+1] << 16 |
			(uint32_t)data[i+2] << 8  |
			(uint32_t)data[i+3]
		);
		if (es_start_code == H264_ANNEXB_START_CODE_SHORT) {
			i += 4;
			got_es_start_code = 1;
		} else {
			es_start_code = (
				0                   << 24 |
				(uint32_t)data[i]   << 16 |
				(uint32_t)data[i+1] << 8  |
				(uint32_t)data[i+2]
			);
			if (es_start_code == H264_ANNEXB_START_CODE_LONG) {
				i += 3;
				got_es_start_code = 1;
			}
		}

		if (got_es_start_code) {
			forbidden_zero_bit = data[i] & 0x80;
			if (forbidden_zero_bit != 0) continue;

			nal_ref_idc = data[i] & 0x60;
			nal_type = data[i] & 0x1F;

			if (
				(nal_type == H264_NAL_TYPE_AUD) ||
				(nal_type == H264_NAL_TYPE_SEI) ||
				(nal_type == H264_NAL_TYPE_SLICE) ||
				(nal_type == H264_NAL_TYPE_IDR) ||
				(nal_type == H264_NAL_TYPE_SPS) ||
				(nal_type == H264_NAL_TYPE_PPS)
			) {
				nal_index++;

				if ((result->len) &&
				    (nal_index != 1))  /* check if it is not first nal in packet; */
				                       /* result->len can be not 0, but offset is 0 */
				                       /* got nal from previous h264_annexb_parse call */
				{ /* sz calculation */
					H264NAL *result_nal_prv = &result->nals[result->len-1];
					size_t offset_prv = (size_t)result->offsets[result->len-1];
					result_nal_prv->sz = (size_t)goffset + (size_t)offset - offset_prv;
				}

				H264NAL *result_nal = &result->nals[result->len];
				result->offsets[result->len] = goffset + (uint64_t)offset;
				result->len++;
				result_nal->sz = datasz - (size_t)offset;

				result_nal->type = nal_type;

				if (nal_type == H264_NAL_TYPE_SPS) {
					h264_nal_sps_parse(&it->nal_sps, &data[i+1]);
					it->got_nal_sps = 1;

					result_nal->u.sps = it->nal_sps;

				} else if (nal_type == H264_NAL_TYPE_PPS) {
					h264_nal_pps_parse(&it->nal_pps, &data[i+1]);
					it->got_nal_pps = 1;

					result_nal->u.pps = it->nal_pps;

				} else if (nal_type == H264_NAL_TYPE_AUD) {
					h264_nal_aud_parse(&it->nal_aud, &data[i+1]);
					it->got_nal_aud = 1;

					result_nal->u.aud = it->nal_aud;

				} else if (nal_type == H264_NAL_TYPE_SEI) {
					h264_nal_sei_parse(&it->nal_sei, &data[i+1]);
					it->got_nal_aud = 1;

					result_nal->u.sei = it->nal_sei;

				} else if ((it->got_nal_sps) &&
				           ((nal_type == H264_NAL_TYPE_SLICE) ||
				            (nal_type == H264_NAL_TYPE_IDR))) {
					h264_nal_slice_idr_parse(&it->nal_slice_idr, &it->nal_sps, &data[i+1]);

					result_nal->u.slice_idr = it->nal_slice_idr;
				}
			} else {
				fprintf(stderr, "[h264 @ %p] got unexpected nal-type %0X\n",
					it, nal_type);
			}
		}
	}

	return ret;
}

void h264_annexb_parse_result_print_humanized_one_line(H264AnnexBParseResult *it) {
	int i = 0;
	const char *nal_type_name = NULL;
	H264NAL *nal = NULL;

	if (!it->len) { return; }

	for (i = 0; i < it->len; i++) {
		nal = &it->nals[i];

		printf("ES 0x%08llX | %5zd | ", (long long unsigned int)it->offsets[i], nal->sz);

		const char *nal_type_name = h264_nal_type_string(nal->type);
		switch (nal->type) {
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
		case H264_NAL_TYPE_SLICE:
		case H264_NAL_TYPE_IDR:
			h264_nal_slice_idr_print_humanized_one_line(&nal->u.slice_idr);
			break;
		}
	}
}
