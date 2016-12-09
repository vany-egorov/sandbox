#ifndef __VA_ATOM_KIND__
#define __VA_ATOM_KIND__


#include "../h264/h264.h"


typedef enum va_atom_kind_enum VAAtomKind;

enum va_atom_kind_enum {
	VA_ATOM_KIND_MPEGTS_HEADER,
	VA_ATOM_KIND_MPEGTS_ADAPTION,
	VA_ATOM_KIND_MPEGTS_PES,
	VA_ATOM_KIND_MPEGTS_PSI_PAT,
	VA_ATOM_KIND_MPEGTS_PSI_PMT,
	VA_ATOM_KIND_MPEGTS_PSI_SDT,

	VA_ATOM_KIND_H264_SPS,
	VA_ATOM_KIND_H264_PPS,
	VA_ATOM_KIND_H264_AUD,
	VA_ATOM_KIND_H264_SEI,
	VA_ATOM_KIND_H264_SLICE_IDR,
};


VAAtomKind va_atom_kind_from_h264_nal_type(H264NALType nal_type);


#endif /* __VA_ATOM_KIND__ */
