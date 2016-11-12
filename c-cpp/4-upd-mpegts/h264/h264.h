#ifndef __H264_H264__
#define __H264_H264__


// h264 containers
// - Annex B
// - AVCC


typedef enum h264_nal_type_enum H264NALType;

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

const char* h264_nal_type_string(H264NALType it);


#endif
