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
// static size_t nal_size = 0;

int h264_annexb_parse(H264 *it, const uint8_t *data, const size_t datasz,
                      H264NAL *out_nals, H264NALType *out_nal_types, int *out_offsets, int *out_len) {
	int ret = 0;
	int i = 0;
	int ii = 0;
	uint32_t es_start_code = 0;
	uint8_t forbidden_zero_bit = 0;
	uint8_t nal_ref_idc = 0;
	H264NALType nal_type = 0;
	int got_es_start_code = 0;

	if (1) {
		if (!is_dump_h264_opened) {
			f_dump_h264 = fopen("./tmp/out.h264", "wb");
			is_dump_h264_opened = 1;
		}
		fwrite(data, datasz, 1, f_dump_h264);
	}

	for (i = 0; i < datasz; i++) {
		ii = i;
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
				H264NAL *out_nal = out_nals;
				H264NALType *out_nal_type = out_nal_types;
				int *out_offset = out_offsets;
				(*out_len)++;
				out_nals++;
				out_nal_types++;
				out_offsets++;

				*out_nal_type = nal_type;
				*out_offset = ii;

				if (nal_type == H264_NAL_TYPE_SPS) {
					h264_nal_sps_parse(&it->nal_sps, &data[i+1]);
					it->got_nal_sps = 1;

					out_nal->sps = it->nal_sps;

				} else if (nal_type == H264_NAL_TYPE_PPS) {
					h264_nal_pps_parse(&it->nal_pps, &data[i+1]);
					it->got_nal_pps = 1;

					out_nal->pps = it->nal_pps;

				} else if (nal_type == H264_NAL_TYPE_AUD) {
					h264_nal_aud_parse(&it->nal_aud, &data[i+1]);
					it->got_nal_aud = 1;

					out_nal->aud = it->nal_aud;

				} else if (nal_type == H264_NAL_TYPE_SEI) {
					h264_nal_sei_parse(&it->nal_sei, &data[i+1]);
					it->got_nal_aud = 1;

					out_nal->sei = it->nal_sei;

				} else if ((it->got_nal_sps) &&
				           ((nal_type == H264_NAL_TYPE_SLICE) ||
				            (nal_type == H264_NAL_TYPE_IDR))) {
					h264_nal_slice_idr_parse(&it->nal_slice_idr, &it->nal_sps, &data[i+1]);

					out_nal->slice_idr = it->nal_slice_idr;
				}
			} else {
				fprintf(stderr, "[h264] got unexpected nal-type %0X\n", nal_type);
			}
		}
	}

	return ret;
}
