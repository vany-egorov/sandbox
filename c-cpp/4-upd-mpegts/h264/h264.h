#ifndef __H264_H264__
#define __H264_H264__


#include <stdio.h>  // printf
#include <stdint.h> // uint8_t, uint32_t


// h264 containers
// - Annex B
// - AVCC
//
// NALU Start Codes
//
// A NALU does not contain is its size.
// Therefore simply concatenating the NALUs to create a stream will not
// work because you will not know where one stops and the next begins.
//
// The Annex B specification solves this by requiring ‘Start Codes’
// to precede each NALU. A start code is 2 or 3 0x00 bytes followed with a 0x01 byte. e.g. 0x000001 or 0x00000001.
//
// The 4 byte variation is useful for transmission over a serial connection as it is trivial to byte align the stream
// by looking for 31 zero bits followed by a one. If the next bit is 0 (because every NALU starts with a 0 bit),
// it is the start of a NALU. The 4 byte variation is usually only used for signaling
// random access points in the stream such as a SPS PPS AUD and IDR
// Where as the 3 byte variation is used everywhere else to save space.
#define H264_ANNEXB_START_CODE_SHORT 0x000001
#define H264_ANNEXB_START_CODE_LONG  0x00000001


typedef enum   h264_nal_type_enum   H264NALType;
typedef struct h264_annex_b_nal_u_s H264AnnexBNALU;

// ITU-T H.264 (V9) (02/2014) - p63
//
// There are 19 different NALU types defined separated into two categories, VCL and non-VCL:
//
// - VCL, or Video Coding Layer packets contain the actual visual information.
// - Non-VCLs contain metadata that may or may not be required to decode the video.
enum h264_nal_type_enum {
	NAL_TYPE_UNSPECIFIED = 0,
	NAL_TYPE_SLICE       = 1,
	NAL_TYPE_DPA         = 2,
	NAL_TYPE_DPB         = 3,
	NAL_TYPE_DPC         = 4,
	// Instantaneous Decoder Refresh (IDR).
	// This VCL NALU is a self contained image slice.
	// That is, an IDR can be decoded and displayed without
	// referencing any other NALU save SPS and PPS.
	NAL_TYPE_IDR         = 5,
	NAL_TYPE_SEI         = 6,
	// Sequence Parameter Set (SPS).
	// This non-VCL NALU contains information required to configure
	// the decoder such as profile, level, resolution, frame rate.
	NAL_TYPE_SPS         = 7,
	// Picture Parameter Set (PPS). Similar to the SPS, this non-VCL
	// contains information on entropy coding mode, slice groups,
	// motion prediction and deblocking filters.
	NAL_TYPE_PPS         = 8,
	// Access Unit Delimiter (AUD).
	// An AUD is an optional NALU that can be use to delimit frames
	// in an elementary stream. It is not required (unless otherwise
	// stated by the container/protocol, like TS), and is often not
	// included in order to save space, but it can be useful to
	// finds the start of a frame without having to fully parse each NALU.
	NAL_TYPE_AUD         = 9,
	NAL_TYPE_EOSEQ       = 10,
	NAL_TYPE_EOSTREAM    = 11,
	NAL_TYPE_FILL        = 12,
	NAL_TYPE_SPS_EXT     = 13,
	NAL_TYPE_PREFIX      = 14,
	NAL_TYPE_SSPS        = 15,
	NAL_TYPE_DPS         = 16,
	NAL_TYPE_CSOACPWP    = 19,
	NAL_TYPE_CSE         = 20,
	NAL_TYPE_CSE3D       = 21,
};

#define NAL_TYPE_UNSPECIFIED_STR "H264 slice"
#define NAL_TYPE_DPA_STR "H264 DPA - Coded slice data partition A"
#define NAL_TYPE_DPB_STR "H264 DPB - Coded slice data partition B"
#define NAL_TYPE_DPC_STR "H264 DPC - Coded slice data partition C"
#define NAL_TYPE_IDR_STR "H264 IDR - instantaneous decoding refresh access-unit/picture"
#define NAL_TYPE_SEI_STR "H264 SEI - Supplemental enhancement information"
#define NAL_TYPE_SPS_STR "H264 SPS - Sequence parameter set"
#define NAL_TYPE_PPS_STR "H264 PPS - Picture parameter set"
#define NAL_TYPE_AUD_STR "H264 AUD - Access unit delimiter"
#define NAL_TYPE_EOSEQ_STR "H264 EOSEQ - end of sequence"
#define NAL_TYPE_EOSTREAM_STR "H264 EOSTREAM - End of stream"
#define NAL_TYPE_FILL_STR "H264 FILL - Filler data"
#define NAL_TYPE_SPS_EXT_STR "H264 SPS EXT - Sequence parameter set extension"
#define NAL_TYPE_PREFIX_STR "H264 PREFIX - Prefix NAL unit"
#define NAL_TYPE_SSPS_STR "H264 SSPS - Subset sequence parameter set"
#define NAL_TYPE_DPS_STR "H264 DPS - Depth parameter set"
#define NAL_TYPE_CSOACPWP_STR "H264 CSOACPWP - Coded slice of an auxiliary" \
                              " coded picture without partitioning"
#define NAL_TYPE_CSE_STR "H264 CSE - Coded slice extension"
#define NAL_TYPE_CSE3D_STR "H264 CSE3D - Coded slice extension for a depth view " \
                           "component or a 3D-AVC texture view component"

struct h264_annex_b_nal_u_s {
	uint8_t slice_type :4;
};

const char* h264_nal_type_string(H264NALType it);
// TODO: remove extra args: app_offset, es_offset, es_length
void        h264_annexb_parse(H264AnnexBNALU *it, const uint8_t *data,
                              uint64_t app_offset, int es_offset, int es_length);


#endif
