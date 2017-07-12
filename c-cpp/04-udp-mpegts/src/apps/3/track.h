#ifndef __APPS_3_TRACK__
#define __APPS_3_TRACK__


#include <common/codec-kind.h>


typedef struct track_s Track;

struct track_s {
	/* index inside Stream */
	uint32_t i;

	/* ID/PID
	 *   - PID for mpegts
	 *   - ID for RTMP/HLS, DASH, MP4
	 */
	uint32_t id;

	/*
	video:
		MPEG-2
		MPEG-4
		H264
		H265
		VP8
		VP9

	audio:
		mp2
		mp3
		aac
		ac3
		vorbis
		opus
	*/
	CodecKind codec_kind;

	/* current global offset
	 * aka bytes-processed / bytes-readen
	 */
	int64_t offset;
};


#endif  /* __APPS_3_TRACK__ */
