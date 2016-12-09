#include "atom-kind.h"


VAAtomKind va_atom_kind_from_h264_nal_type(H264NALType nal_type) {
	switch (nal_type) {
		case H264_NAL_TYPE_SPS: return VA_ATOM_KIND_H264_SPS;
		case H264_NAL_TYPE_PPS: return VA_ATOM_KIND_H264_PPS;
		case H264_NAL_TYPE_AUD: return VA_ATOM_KIND_H264_AUD;
		case H264_NAL_TYPE_SEI: return VA_ATOM_KIND_H264_SEI;
		case H264_NAL_TYPE_SLICE:
		case H264_NAL_TYPE_IDR: return VA_ATOM_KIND_H264_SLICE_IDR;
		default:
			return H264_NAL_TYPE_UNSPECIFIED;
	}

	return H264_NAL_TYPE_UNSPECIFIED;
}
