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


typedef enum   h264_nal_type_enum         H264NALType;
typedef struct h264_s                     H264;
typedef enum   h264_nal_slice_type_enum   H264NALSliceType;
typedef struct h264_nal_s                 H264NAL;
typedef struct h264_nal_sps_s             H264NALSPS;
typedef struct h264_nal_pps_s             H264NALPPS;
typedef struct h264_nal_aud_s             H264NALAUD;
typedef struct h264_nal_sei_s             H264NALSEI;
typedef struct h264_nal_slice_idr_s       H264NALSliceIDR;
typedef struct h264_annexb_parse_result_s H264AnnexBParseResult;

// ITU-T H.264 (V9) (02/2014) - p63
//
// There are 19 different NALU types defined separated into two categories, VCL and non-VCL:
//
// - VCL, or Video Coding Layer packets contain the actual visual information.
// - Non-VCLs contain metadata that may or may not be required to decode the video.
enum h264_nal_type_enum {
	H264_NAL_TYPE_UNSPECIFIED = 0,
	H264_NAL_TYPE_SLICE       = 1,
	H264_NAL_TYPE_DPA         = 2,
	H264_NAL_TYPE_DPB         = 3,
	H264_NAL_TYPE_DPC         = 4,
	// Instantaneous Decoder Refresh (IDR).
	// This VCL NALU is a self contained image slice.
	// That is, an IDR can be decoded and displayed without
	// referencing any other NALU save SPS and PPS.
	H264_NAL_TYPE_IDR         = 5,
	H264_NAL_TYPE_SEI         = 6,
	// Sequence Parameter Set (SPS).
	// This non-VCL NALU contains information required to configure
	// the decoder such as profile, level, resolution, frame rate.
	H264_NAL_TYPE_SPS         = 7,
	// Picture Parameter Set (PPS). Similar to the SPS, this non-VCL
	// contains information on entropy coding mode, slice groups,
	// motion prediction and deblocking filters.
	H264_NAL_TYPE_PPS         = 8,
	// Access Unit Delimiter (AUD).
	// An AUD is an optional NALU that can be use to delimit frames
	// in an elementary stream. It is not required (unless otherwise
	// stated by the container/protocol, like TS), and is often not
	// included in order to save space, but it can be useful to
	// finds the start of a frame without having to fully parse each NALU.
	H264_NAL_TYPE_AUD         = 9,
	H264_NAL_TYPE_EOSEQ       = 10,
	H264_NAL_TYPE_EOSTREAM    = 11,
	H264_NAL_TYPE_FILL        = 12,
	H264_NAL_TYPE_SPS_EXT     = 13,
	H264_NAL_TYPE_PREFIX      = 14,
	H264_NAL_TYPE_SSPS        = 15,
	H264_NAL_TYPE_DPS         = 16,
	H264_NAL_TYPE_CSOACPWP    = 19,
	H264_NAL_TYPE_CSE         = 20,
	H264_NAL_TYPE_CSE3D       = 21,
};

#define H264_NAL_TYPE_UNSPECIFIED_STR "H264 slice"
#define H264_NAL_TYPE_DPA_STR "H264 DPA - Coded slice data partition A"
#define H264_NAL_TYPE_DPB_STR "H264 DPB - Coded slice data partition B"
#define H264_NAL_TYPE_DPC_STR "H264 DPC - Coded slice data partition C"
#define H264_NAL_TYPE_IDR_STR "H264 IDR - instantaneous decoding refresh access-unit/picture"
#define H264_NAL_TYPE_SEI_STR "H264 SEI - Supplemental enhancement information"
#define H264_NAL_TYPE_SPS_STR "H264 SPS - Sequence parameter set"
#define H264_NAL_TYPE_PPS_STR "H264 PPS - Picture parameter set"
#define H264_NAL_TYPE_AUD_STR "H264 AUD - Access unit delimiter"
#define H264_NAL_TYPE_EOSEQ_STR "H264 EOSEQ - end of sequence"
#define H264_NAL_TYPE_EOSTREAM_STR "H264 EOSTREAM - End of stream"
#define H264_NAL_TYPE_FILL_STR "H264 FILL - Filler data"
#define H264_NAL_TYPE_SPS_EXT_STR "H264 SPS EXT - Sequence parameter set extension"
#define H264_NAL_TYPE_PREFIX_STR "H264 PREFIX - Prefix NAL unit"
#define H264_NAL_TYPE_SSPS_STR "H264 SSPS - Subset sequence parameter set"
#define H264_NAL_TYPE_DPS_STR "H264 DPS - Depth parameter set"
#define H264_NAL_TYPE_CSOACPWP_STR "H264 CSOACPWP - Coded slice of an auxiliary" \
                              " coded picture without partitioning"
#define H264_NAL_TYPE_CSE_STR "H264 CSE - Coded slice extension"
#define H264_NAL_TYPE_CSE3D_STR "H264 CSE3D - Coded slice extension for a depth view " \
                           "component or a 3D-AVC texture view component"


/* bitstream.c */
inline uint32_t h264_bitstream_get_bits(const uint8_t * const base, uint32_t * const offset, uint8_t bits);
uint32_t bitstream_decode_u_golomb(const uint8_t * const base, uint32_t * const offset);


/* nal-sps.c */
struct h264_nal_sps_s {
	uint8_t profile_idc;
	uint8_t
		constraint_set0_flag :1,
		constraint_set1_flag :1,
		constraint_set2_flag :1,
		constraint_set3_flag :1,
		constraint_set4_flag :1,
		constraint_set5_flag :1,
		reserved_zero_2bits  :2;
	uint8_t level_idc;
	uint8_t seq_parameter_set_id;
	uint8_t chroma_format_idc;
	uint8_t
		separate_colour_plane_flag           :1,
		qpprime_y_zero_transform_bypass_flag :1,
		seq_scaling_matrix_present_flag      :1,
		pic_order_cnt_type                   :1,
		delta_pic_order_always_zero_flag     :1,
		gaps_in_frame_num_value_allowed_flag :1,
		frame_mbs_only_flag                  :1;
	uint8_t bit_depth_luma_minus8;
	uint8_t bit_depth_chroma_minus8;
	uint8_t log2_max_frame_num_minus4;
	uint8_t log2_max_pic_order_cnt_lsb_minus4;
	int8_t  offset_for_non_ref_pic;
	int8_t  offset_for_top_to_bottom_field;
	uint8_t num_ref_frames_in_pic_order_cnt_cycle;
	uint8_t max_num_ref_frames;
	uint16_t pic_width_in_mbs_minus1;
	uint16_t pic_height_in_map_units_minus1;
};

int  h264_nal_sps_parse(H264NALSPS *it, const uint8_t *data);
void h264_nal_sps_print_json(H264NALSPS *it);
void h264_nal_sps_print_humanized(H264NALSPS *it);


/* nal-pps.c */
struct h264_nal_pps_s {
	uint16_t pic_parameter_set_id;
	uint16_t seq_parameter_set_id;
	uint8_t
		entropy_coding_mode_flag                     :1,
		bottom_field_pic_order_in_frame_present_flag :1,
		reserved_bit_fields                          :6;
	uint16_t num_slice_groups_minus1;
};

int  h264_nal_pps_parse(H264NALPPS *it, const uint8_t *data);
void h264_nal_pps_print_json(H264NALPPS *it);
void h264_nal_pps_print_humanized(H264NALPPS *it);


/* nal-aud.c */
struct h264_nal_aud_s {
	uint8_t primary_pic_type;
};

int  h264_nal_aud_parse(H264NALAUD *it, const uint8_t *data);
void h264_nal_aud_print_json(H264NALAUD *it);
void h264_nal_aud_print_humanized(H264NALAUD *it);

/* nal-sei.c */
struct h264_nal_sei_s {
	uint8_t reserved;
};

int  h264_nal_sei_parse(H264NALSEI *it, const uint8_t *data);
void h264_nal_sei_print_json(H264NALSEI *it);
void h264_nal_sei_print_humanized(H264NALSEI *it);


/* nal-slice-idr.c */
enum h264_nal_slice_type_enum {
	/// Consists of P-macroblocks
	//  (each macro block is predicted using
	//  one reference frame) and / or I-macroblocks.
	H264_NAL_SLICE_TYPE_P   = 0,
	// Consists of B-macroblocks (each
	// macroblock is predicted using one or two
	// reference frames) and / or I-macroblocks.
	H264_NAL_SLICE_TYPE_B   = 1,
	// Contains only I-macroblocks.
	// Each macroblock is predicted from previously
	// coded blocks of the same slice.
	H264_NAL_SLICE_TYPE_I   = 2,
	// SP-slice. Consists of P and / or I-macroblocks
	// and lets you switch between encoded streams.
	H264_NAL_SLICE_TYPE_SP  = 3,
	// SI-slice. It consists of a special type
	// of SI-macroblocks and lets you switch between
	// encoded streams.
	H264_NAL_SLICE_TYPE_SI  = 4,

	H264_NAL_SLICE_TYPE_P2  = 5,
	H264_NAL_SLICE_TYPE_B2  = 6,
	H264_NAL_SLICE_TYPE_I2  = 7,
	H264_NAL_SLICE_TYPE_SP2 = 8,
	H264_NAL_SLICE_TYPE_SI2 = 9,
};

struct h264_nal_slice_idr_s {
	H264NALSliceType slice_type;

	uint8_t
		first_mb_in_slice   :1,
		field_pic_flag      :1,
		bottom_field_flag   :1,
		reserved_bit_fields :5;
	uint16_t pic_parameter_set_id;
	uint16_t frame_num;
	uint16_t pic_order_cnt_lsb;
};

int  h264_nal_slice_idr_parse(H264NALSliceIDR *it, H264NALSPS *sps, const uint8_t *data);
void h264_nal_slice_idr_print_humanized_one_line(H264NALSliceIDR *it); /* debug */


/* h264.c */
struct h264_nal_s {
	H264NALType type;
	size_t      sz;  /* TODO: move to h264_annexb_parse_result_s? as szs[7] */

	union {
		H264NALSPS      sps;
		H264NALPPS      pps;
		H264NALAUD      aud;
		H264NALSEI      sei;
		H264NALSliceIDR slice_idr;
	} u; /* NAL unit */
};

struct h264_s {
	uint8_t
		got_nal_sps         :1,
		got_nal_pps         :1,
		got_nal_aud         :1,
		reserved_bit_fields :5;

	H264NALSPS nal_sps;
	H264NALPPS nal_pps;
	H264NALAUD nal_aud;
	H264NALSEI nal_sei;

	H264NALSliceIDR nal_slice_idr;
};

/* must be set to { 0 } */
struct h264_annexb_parse_result_s {
	int      len;         /* total nals parsed count */
	H264NAL  nals[7];     /* NALs with type */
	uint64_t offsets[7];  /* position of each NALu inside stream (offset from beginning) */
};

const char* h264_nal_type_string(H264NALType it);
int         h264_annexb_parse(H264 *it, const uint8_t *data, const size_t datasz, const uint64_t goffset, H264AnnexBParseResult *result);
void        h264_annexb_parse_result_print_humanized_one_line(H264AnnexBParseResult *it); /* debug */


#endif
