#include "fn.h"


CodecKind fn_codec_kind_from_mpegts_pmt_pe(MPEGTSPSIPMTProgramElement *pe) {
	MPEGTSESType st = pe->stream_type;
	MPEGTSPSIPMTESInfo *ei = NULL;
	MPEGTSPSIPEDTag tag = 0;

	switch (st) {
	case MPEGTS_STREAM_TYPE_AUDIO_MPEG1: return CODEC_KIND_MP2;
	case MPEGTS_STREAM_TYPE_AUDIO_MPEG2: return CODEC_KIND_MP2;
	case MPEGTS_STREAM_TYPE_AUDIO_AAC_ADTS: return CODEC_KIND_AAC;
	case MPEGTS_STREAM_TYPE_AUDIO_AC3: return CODEC_KIND_AC3;

	case MPEGTS_STREAM_TYPE_VIDEO_MPEG2: return CODEC_KIND_MPEG_2;
	case MPEGTS_STREAM_TYPE_VIDEO_H264: return CODEC_KIND_H264;
	case MPEGTS_STREAM_TYPE_VIDEO_H265: return CODEC_KIND_H265;
	default: break;
	}

	if (pe->es_infos.c) {
		{int i = 0; for (i = 0; i < pe->es_infos.len; i++) {
			ei = &pe->es_infos.c[i];
			tag = ei->descriptor_tag;

			switch (tag) {
			case MPEGTS_PSI_PED_TAG_AC3: return CODEC_KIND_AC3;
			case MPEGTS_PSI_PED_TAG_EBU_TELETEXT: return CODEC_KIND_TELETEXT;
			default:
				break;
			}
		}}
	}

	return CODEC_KIND_UNKNOWN;
}
