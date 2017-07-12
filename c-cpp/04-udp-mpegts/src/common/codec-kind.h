#ifndef __COMMON_CODEC_KIND__
#define __COMMON_CODEC_KIND__


#include <mpegts/mpegts.h>  /* MPEGTSESType */


#define CODEC_KIND_UNKNOWN_STR "unknown"

#define CODEC_KIND_VIDEO_STR  "video"
#define CODEC_KIND_MPEG_2_STR "MPEG-2"
#define CODEC_KIND_MPEG_4_STR "MPEG-4"
#define CODEC_KIND_H264_STR   "H.264/MPEG-4 Part 10/AVC"
#define CODEC_KIND_H265_STR   "H.265/HEVC"
#define CODEC_KIND_VP8_STR    "VP8"
#define CODEC_KIND_VP9_STR    "VP9"

#define CODEC_KIND_AUDIO_STR  "audio"
#define CODEC_KIND_MP2_STR    "mp2/MPEG-1 Audio Layer II/Musicam"
#define CODEC_KIND_MP3_STR    "mp3/MPEG-1/2/2.5 Layer 3"
#define CODEC_KIND_AAC_STR    "AAC/Advanced Audio Coding"
#define CODEC_KIND_AC3_STR    "AC-3/Dolby Digital"
#define CODEC_KIND_VORBIS_STR "vorbis"
#define CODEC_KIND_OPUS_STR   "opus"


typedef enum codec_kind_enum {
	CODEC_KIND_UNKNOWN,

	CODEC_KIND_VIDEO = 0x80,
	CODEC_KIND_MPEG_2 = CODEC_KIND_VIDEO | 1,
	CODEC_KIND_MPEG_4,
	CODEC_KIND_H264,
	CODEC_KIND_H265,
	CODEC_KIND_VP8,
	CODEC_KIND_VP9,

	CODEC_KIND_AUDIO = 0x40,
	CODEC_KIND_MP2 = CODEC_KIND_AUDIO | 1,
	CODEC_KIND_MP3,
	CODEC_KIND_AAC,
	CODEC_KIND_AC3,
	CODEC_KIND_VORBIS,
	CODEC_KIND_OPUS,
} CodecKind;


char *codec_kind_str(CodecKind it);
CodecKind codec_kind_from_mpegts_es_type(MPEGTSESType v);


#endif // __COMMON_CODEC_KIND__
