#include "codec-kind.h"


char *codec_kind_str(CodecKind it) {
	switch (it) {
	case CODEC_KIND_VIDEO: return CODEC_KIND_VIDEO_STR;
	case CODEC_KIND_MPEG_2: return CODEC_KIND_MPEG_2_STR;
	case CODEC_KIND_MPEG_4: return CODEC_KIND_MPEG_4_STR;
	case CODEC_KIND_H264: return CODEC_KIND_H264_STR;
	case CODEC_KIND_H265: return CODEC_KIND_H265_STR;
	case CODEC_KIND_VP8: return CODEC_KIND_VP8_STR;
	case CODEC_KIND_VP9: return CODEC_KIND_VP9_STR;

	case CODEC_KIND_AUDIO: return CODEC_KIND_AUDIO_STR;
	case CODEC_KIND_MP2: return CODEC_KIND_MP2_STR;
	case CODEC_KIND_MP3: return CODEC_KIND_MP3_STR;
	case CODEC_KIND_AAC: return CODEC_KIND_AAC_STR;
	case CODEC_KIND_AC3: return CODEC_KIND_AC3_STR;
	case CODEC_KIND_VORBIS: return CODEC_KIND_VORBIS_STR;
	case CODEC_KIND_OPUS: return CODEC_KIND_OPUS_STR;

	case CODEC_KIND_TELETEXT: return CODEC_KIND_TELETEXT_STR;
	case CODEC_KIND_SUBTITLE: return CODEC_KIND_SUBTITLE_STR;

	default: return CODEC_KIND_UNKNOWN_STR;
	}

	return CODEC_KIND_UNKNOWN_STR;
}
