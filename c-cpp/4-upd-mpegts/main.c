#include <stdio.h>
#include <sysexits.h>   // EX_OK, EX_SOFTWARE
#include <inttypes.h>
#include <unistd.h>     // close
#include <fcntl.h>      // open
#include <string.h>     // memcpy, memset, size_t
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>

#include "./mpegts/mpegts.h"
#include "url.h"
#include "fifo.h"
#include "color.h"
#include "config.h"


// h264 containers
// - Annex B
// - AVCC

#define EXAMPLE_PORT 5500
#define EXAMPLE_GROUP "239.1.1.1"
#define MSG_SIZE 1504

#define MPEGTS_SYNC_BYTE    0x47
#define MPEGTS_PACKET_SIZE  188

#define MPEGTS_PID_PAT      0x0000
#define MPEGTS_PID_CAT      0x0001
#define MPEGTS_PID_TSDT     0x0002
#define MPEGTS_PID_CIT      0x0003
#define MPEGTS_PID_SDT      0x0011
#define MPEGTS_PID_NULL     0x1FFF

#define PES_START_CODE 0x000001

#define PES_PTS_DTS_INDICATOR_NO        0b00
#define PES_PTS_DTS_INDICATOR_FORBIDDEN 0b01
#define PES_PTS_DTS_INDICATOR_PTS       0b10
#define PES_PTS_DTS_INDICATOR_PTS_DTS   0b11

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
#define ES_START_CODE_SHORT 0x000001
#define ES_START_CODE_LONG  0x00000001

// ITU-T H.264 (V9) (02/2014) - p63
//
// There are 19 different NALU types defined separated into two categories, VCL and non-VCL:
//
// - VCL, or Video Coding Layer packets contain the actual visual information.
// - Non-VCLs contain metadata that may or may not be required to decode the video.
typedef enum {
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
} NALType;

static void nal_type_str(NALType v, char *out) {
	switch(v) {
	case NAL_TYPE_SLICE:
		strcpy(out, "H264 slice");
		break;
	case NAL_TYPE_DPA:
		strcpy(out, "H264 DPA - Coded slice data partition A");
		break;
	case NAL_TYPE_DPB:
		strcpy(out, "H264 DPB - Coded slice data partition B");
		break;
	case NAL_TYPE_DPC:
		strcpy(out, "H264 DPC - Coded slice data partition C");
		break;
	case NAL_TYPE_IDR:
		strcpy(out, "H264 IDR - instantaneous decoding refresh access-unit/picture");
		break;
	case NAL_TYPE_SEI:
		strcpy(out, "H264 SEI - Supplemental enhancement information");
		break;
	case NAL_TYPE_SPS:
		strcpy(out, "H264 SPS - Sequence parameter set");
		break;
	case NAL_TYPE_PPS:
		strcpy(out, "H264 PPS - Picture parameter set");
		break;
	case NAL_TYPE_AUD:
		strcpy(out, "H264 AUD - Access unit delimiter");
		break;
	case NAL_TYPE_EOSEQ:
		strcpy(out, "H264 EOSEQ - end of sequence");
		break;
	case NAL_TYPE_EOSTREAM:
		strcpy(out, "H264 EOSTREAM - End of stream");
		break;
	case NAL_TYPE_FILL:
		strcpy(out, "H264 FILL - Filler data");
		break;
	case NAL_TYPE_SPS_EXT:
		strcpy(out, "H264 SPS EXT - Sequence parameter set extension");
		break;
	case NAL_TYPE_PREFIX:
		strcpy(out, "H264 PREFIX - Prefix NAL unit");
		break;
	case NAL_TYPE_SSPS:
		strcpy(out, "H264 SSPS - Subset sequence parameter set");
		break;
	case NAL_TYPE_DPS:
		strcpy(out, "H264 DPS - Depth parameter set");
		break;
	case NAL_TYPE_CSOACPWP:
		strcpy(out, "H264 CSOACPWP - Coded slice of an auxiliary "
								"coded picture without partitioning");
		break;
	case NAL_TYPE_CSE:
		strcpy(out, "H264 CSE - Coded slice extension");
		break;
	case NAL_TYPE_CSE3D:
		strcpy(out, "H264 CSE3D - Coded slice extension for a depth view "
								"component or a 3D-AVC texture view component");
		break;
	}
}

inline uint32_t get_bit(const uint8_t * const base, uint32_t offset) {
	return ((*(base + (offset >> 0x3))) >> (0x7 - (offset & 0x7))) & 0x1;
}

inline uint32_t get_bits(const uint8_t * const base, uint32_t * const offset, uint8_t bits) {
	int i = 0;
	uint32_t value = 0;
	for (i = 0; i < bits; i++) {
		value = (value << 1) | (get_bit(base, (*offset)++) ? 1 : 0);
	}
	return value;
}

// This function implement decoding of exp-Golomb codes of zero range (used in H.264).
uint32_t decode_u_golomb(const uint8_t * const base, uint32_t * const offset) {
	uint32_t zeros = 0;

	// calculate zero bits. Will be optimized.
	while (0 == get_bit(base, (*offset)++)) zeros++;

	// insert first 1 bit
	uint32_t info = 1 << zeros;

	int32_t i = 0;
	for (i = zeros - 1; i >= 0; i--) {
		info |= get_bit(base, (*offset)++) << i;
	}

	return (info - 1);
}

typedef struct App {
	FIFO    *fifo;
	uint64_t offset; // global offset
	uint16_t program_map_PID;
	uint16_t video_PID_H264;
} App;

static App *app_new(void) {
	App *it = (App*)calloc(1, sizeof(App));
	it->program_map_PID = 0;
	it->video_PID_H264 = 0;
}

static FILE *f_dump_h264 = NULL;
static int is_dump_h264_opened = 0;
static int nal_size = 0;

// TODO: remove extra args: app_offset, es_offset, es_length
static void es_parse(uint8_t *data, uint64_t app_offset, int es_offset, int es_length) {
	int es_i = 0;
	uint64_t es_offset_current = 0;
	uint32_t es_start_code = 0;
	int got_es_start_code = 0;

	if (1) {
		if (!is_dump_h264_opened) {
			f_dump_h264 = fopen("./tmp/out.h264", "wb");
			is_dump_h264_opened = 1;
		}
		nal_size += es_length;
		fwrite(data, es_length, 1, f_dump_h264);
	}

	for (es_i = 0; es_i < es_length; es_i++) {
		es_start_code = 0;
		got_es_start_code = 0;

		es_offset_current = app_offset + es_offset + es_i;

		es_start_code = (
			(uint32_t)data[es_i]   << 24 |
			(uint32_t)data[es_i+1] << 16 |
			(uint32_t)data[es_i+2] << 8  |
			(uint32_t)data[es_i+3]
		);
		if (es_start_code == ES_START_CODE_LONG) {
			es_i += 4;
			got_es_start_code = 1;
		} else {
			es_start_code = (
				0                      << 24 |
				(uint32_t)data[es_i]   << 16 |
				(uint32_t)data[es_i+1] << 8  |
				(uint32_t)data[es_i+2]
			);
			if (es_start_code == ES_START_CODE_SHORT) {
				es_i += 3;
				got_es_start_code = 1;
			}
		}

		if (got_es_start_code) {
			uint8_t forbidden_zero_bit = data[es_i] & 0x80;
			if (forbidden_zero_bit != 0) continue;

			uint8_t nal_ref_idc = data[es_i] & 0x60;
			uint8_t nal_type = data[es_i] & 0x1F;

			char nal_type_name[255] = { 0 };
			nal_type_str(nal_type, nal_type_name);

			if (
				(nal_type == NAL_TYPE_AUD) ||
				(nal_type == NAL_TYPE_SEI) ||
				(nal_type == NAL_TYPE_SLICE) ||
				(nal_type == NAL_TYPE_IDR) ||
				(nal_type == NAL_TYPE_SPS) ||
				(nal_type == NAL_TYPE_PPS)
			) {
				switch (nal_type) {
				case NAL_TYPE_AUD:
					printf("ES 0x%08llX | " COLOR_BRIGHT_YELLOW "%s" COLOR_RESET "\n",
						es_offset_current, nal_type_name);
					break;
				case NAL_TYPE_SEI:
					printf("ES 0x%08llX | " COLOR_BRIGHT_BLUE "%s" COLOR_RESET "\n",
						es_offset_current, nal_type_name);
					break;
				case NAL_TYPE_SPS:
					printf("ES 0x%08llX | " COLOR_BRIGHT_WHITE "%s" COLOR_RESET "\n",
						es_offset_current, nal_type_name);
					break;
				case NAL_TYPE_PPS:
					printf("ES 0x%08llX | " COLOR_BRIGHT_WHITE "%s" COLOR_RESET "\n",
						es_offset_current, nal_type_name);
					break;
				}

				// slice hader
				if ((nal_type == NAL_TYPE_SLICE) || (nal_type == NAL_TYPE_IDR)) {
					uint32_t offset = 0;
					// printf(">>> 1 - %d\n", offset);
					uint32_t first_mb_in_slice = decode_u_golomb(&data[es_i+1], &offset);
					// printf(">>> 2 - %d\n", offset);
					uint32_t slice_type = decode_u_golomb(&data[es_i+1], &offset);
					// printf(">>> 3 - %d\n", offset);
					uint32_t pic_parameter_set_id = decode_u_golomb(&data[es_i+1], &offset);
					// printf(">>> 4 - %d\n", offset);
					uint32_t frame_num = decode_u_golomb(&data[es_i+1], &offset);
					// printf(">>> 5 - %d\n", offset);
					uint32_t pic_order_cnt_lsb = decode_u_golomb(&data[es_i+1], &offset);
					// printf(">>> 6 - %d\n", offset);

					// 0 => P-slice. Consists of P-macroblocks
					//      (each macro block is predicted using
					//      one reference frame) and / or I-macroblocks.
					// 1 => B-slice. Consists of B-macroblocks (each
					//      macroblock is predicted using one or two
					//      reference frames) and / or I-macroblocks.
					// 2 => I-slice. Contains only I-macroblocks.
					//      Each macroblock is predicted from previously
					//      coded blocks of the same slice.
					// 3 => SP-slice. Consists of P and / or I-macroblocks
					//      and lets you switch between encoded streams.
					// 4 => SI-slice. It consists of a special type
					//      of SI-macroblocks and lets you switch between
					//      encoded streams.
					// 5 => P-slice.
					// 6 => B-slice.
					// 7 => I-slice.
					// 8 => SP-slice.
					// 9 => SI-slice.
					switch (slice_type) {
					case 0:
					case 5:
						printf("ES 0x%08llX | %d | " COLOR_BRIGHT_CYAN "H264 P slice #%d { frame-num: %d, pic-order-cnt-lsb: %d }" COLOR_RESET "\n",
							es_offset_current, nal_size, 0, frame_num, pic_order_cnt_lsb);
						break;
					case 1:
					case 6:
						printf("ES 0x%08llX | %d | " COLOR_BRIGHT_GREEN "H264 B slice #%d { frame-num: %d, pic-order-cnt-lsb: %d }" COLOR_RESET "\n",
							es_offset_current, nal_size, 0, frame_num, pic_order_cnt_lsb);
						break;
					case 2:
					case 7:
						printf("ES 0x%08llX | %d | " COLOR_BRIGHT_RED "H264 I slice #%d { frame-num: %d, pic-order-cnt-lsb: %d }" COLOR_RESET "\n",
							es_offset_current, nal_size, 0, frame_num, pic_order_cnt_lsb);
						break;
					default:
						printf("ES 0x%08llX | " COLOR_BRIGHT_RED "H264 slice #%d { slice-type: %d }" COLOR_RESET "\n",
							es_offset_current, frame_num, slice_type);
						break;
					}

					nal_size = 0;
				}
			} else {
				// printf("!! ES 0x%08llX %d\n", es_offset_current, nal_type);
			}
		}
	}
}

struct PES_ {
	uint64_t PTS;
	uint64_t DTS;
};
typedef struct PES_ PES;

// TODO: remove extra args: app_offset, pes_offset, i
static void pes_parse(PES *it, uint8_t *data, uint64_t app_offset, uint64_t pes_offset, int i) {
	uint32_t start_code = (
		0 << 24                 |
		(uint32_t)data[0] << 16 |
		(uint32_t)data[1] << 8  |
		(uint32_t)data[2]
	);
	// http://dvd.sourceforge.net/dvdinfo/pes-hdr.html
	if (start_code == PES_START_CODE) {
		uint8_t stream_id = (uint8_t)data[3];
		uint16_t pes_packet_length = (
			(uint16_t)data[4] & 0xFF << 8 |
			(uint16_t)data[5] & 0xFF
		);

		// PES header
		// 10 binary or 0x2 hex
		uint8_t marker_bits = (((uint8_t)data[6] & 0xC0) >> 6);
		// 00 - not scrambled
		uint8_t scrambling_control = (uint8_t)data[6] & 0x30;
		uint8_t priority = !!( (uint8_t)data[6] & 0x08 );
		// 1 indicates that the PES packet
		// header is immediately followed by
		// the video start code or audio syncword
		uint8_t data_alignment_indicator = !!( (uint8_t)data[6] & 0x04 );
		uint8_t copyright = !!( (uint8_t)data[6] & 0x02 );
		uint8_t original_or_copy = !!( (uint8_t)data[6] & 0x01 );
		// 11 = both present;
		// 01 is forbidden;
		// 10 = only PTS;
		// 00 = no PTS or DTS
		uint8_t PTS_DTS_indicator = (((uint8_t)data[7] & 0xC0) >> 6);
		// This is the Elementary Stream Clock Reference,
		// used if the stream and system levels are not synchronized'
		// (i.e. ESCR differs from SCR in the PACK header).
		uint8_t ESCR_flag = (uint8_t)data[7] & 0x20;
		// The rate at which data is delivered for this stream,
		// in units of 50 bytes/second.
		uint8_t ES_rate_flag = (uint8_t)data[7] & 0x10;
		uint8_t DSM_trick_mode_flag = (uint8_t)data[7] & 0x08;
		uint8_t additional_copy_info_flag = (uint8_t)data[7] & 0x04;
		uint8_t CRC_flag = (uint8_t)data[7] & 0x02;
		uint8_t extension_flag = (uint8_t)data[7] & 0x01;
		uint8_t PES_header_length = (uint8_t)data[8];
		// printf("%d | %d | 0x%02x\n", data_alignment_indicator, PES_header_length, PTS_DTS_indicator);
		if ((PTS_DTS_indicator == PES_PTS_DTS_INDICATOR_PTS) ||
			  (PTS_DTS_indicator == PES_PTS_DTS_INDICATOR_PTS_DTS)) {
			uint64_t PTS = (
				(((uint64_t)data[9]  & 0x0E) << 32) |
				(((uint64_t)data[10] & 0xFF) << 24) |
				(((uint64_t)data[11] & 0xFE) << 16) |
				(((uint64_t)data[12] & 0xFF) << 8) |
				 ((uint64_t)data[13] & 0xFE)
			);
			uint64_t DTS = 0;
			if (PTS_DTS_indicator == PES_PTS_DTS_INDICATOR_PTS_DTS) {
				DTS = (
					(((uint64_t)data[14]  & 0x0E) << 32) |
					(((uint64_t)data[15] & 0xFF) << 24) |
					(((uint64_t)data[16] & 0xFE) << 16) |
					(((uint64_t)data[17] & 0xFF) << 8) |
					 ((uint64_t)data[18] & 0xFE)
				);
			}

			it->PTS = PTS;
			it->DTS = DTS;
		}

		uint8_t *es_data = &data[9+PES_header_length];
		int es_offset = pes_offset + PES_header_length + 9;
		int es_length = MPEGTS_PACKET_SIZE - es_offset;

		printf("PES offset G 0x%" PRIX64 " | PES offset 0x%" PRIX64 " | ES offset 0x%" PRIX64 " \n",
			app_offset,
			app_offset + pes_offset,
			app_offset + es_offset);

		es_parse(es_data, app_offset, es_offset, es_length);
	}
}

void on_msg(App *app, uint8_t *msg) {
	int i = 0;
	MPEGTSHeader mpegts_header = { 0 };
	MPEGTSAdaption mpegts_adaption = { 0 };
	MPEGTSPSI mpegts_psi = { 0 };
	MPEGTSPAT mpegts_pat = { 0 };

	if (msg[0] != MPEGTS_SYNC_BYTE) return;

	mpegts_header_parse(&mpegts_header, &msg[i+1]);

	if (mpegts_header.adaption_field_control) {
		mpegts_adaption_parse(&mpegts_adaption, &msg[i+4]);

		if (mpegts_adaption.PCR_flag)
			mpegts_pcr_print_json(&mpegts_adaption.PCR);
	}

	if (mpegts_header.PID == MPEGTS_PID_PAT) {
		mpegts_psi_parse(&mpegts_psi, &msg[i+5]);
		// printf("PAT: "); psi_print(&psi);

		if (mpegts_psi.table_id == MPEGTS_TABLE_ID_PROGRAM_ASSOCIATION_SECTION) {
			mpegts_pat_parse(&mpegts_pat, &msg[i+13]);
			app->program_map_PID = mpegts_pat.program_map_PID;
		}
	} else if ((app->program_map_PID) && (mpegts_header.PID == app->program_map_PID)) {
		mpegts_psi_parse(&mpegts_psi, &msg[i+5]);
		// printf("PMT: "); psi_print(&psi);

		if (mpegts_psi.table_id == MPEGTS_TABLE_ID_PROGRAM_MAP_SECTION) {
			MPEGTSPMT mpegts_pmt = {0};
			mpegts_pmt_parse(&mpegts_pmt, &mpegts_psi, &msg[i+13]);

			int16_t section_length_unreaded = (int16_t)mpegts_psi.section_length;
			// transport-stream-id   x2
			// version-number        x1
			// curent-next-indicator
			// section-number        x1
			// last-section-number   x1
			// CRC32                 x4
			//
			// -----                 x9
			section_length_unreaded -= 9;

			uint16_t PCR_PID = (
				(((uint16_t)msg[i+13] & 0x1F) << 8) |
				((uint16_t)msg[i+14] & 0xFF)
			);
			section_length_unreaded -= 2;

			uint16_t program_info_length = (
				(((uint16_t)msg[i+15] & 0x03) << 8) |
				((uint16_t)msg[i+16] & 0xFF)
			);
			section_length_unreaded -= 2;
			// printf("%d | %d | %d\n", PCR_PID, program_info_length, section_length_unreaded);

			int pmt_start = i + 17;
			int pmt_offset = 0;
			while (section_length_unreaded > 0) {
				uint8_t stream_type = (uint8_t)msg[pmt_start+pmt_offset];
				uint16_t elementary_PID = (
					(((uint16_t)msg[pmt_start+pmt_offset+1] & 0x1F) << 8) |
					((uint16_t)msg[pmt_start+pmt_offset+2] & 0xFF)
				);
				uint16_t ES_info_length = (
					(((uint16_t)msg[pmt_start+pmt_offset+3] & 0x03) << 8) |
					((uint16_t)msg[pmt_start+pmt_offset+4] & 0xFF)
				);

				printf("%s\n", mpegts_es_type_string(stream_type));
				// printf("\t - 0x%02x | 0x%02x | %d | %d | %s\n", stream_type, elementary_PID, ES_info_length, psi.section_length, stream_type_name);

				int ES_info_start = pmt_start+pmt_offset+5;
				int ES_info_offset = 0;
				int16_t ES_info_length_unreaded = ES_info_length;
				while (ES_info_length_unreaded > 0) {
					uint8_t descriptor_tag = (uint8_t)msg[ES_info_start+ES_info_offset];
					uint8_t descriptor_length = (uint8_t)msg[ES_info_start+ES_info_offset+1];
					printf("\t\t - 0x%02x | %d\n", descriptor_tag, descriptor_length);
					ES_info_length_unreaded -= (2 + descriptor_length);
					descriptor_length += (2 + descriptor_length);
				}

				section_length_unreaded -= (5 + ES_info_length);
				pmt_offset +=	(5 + ES_info_length);

				if (stream_type == MPEGTS_STREAM_TYPE_VIDEO_H264)
					app->video_PID_H264 = elementary_PID;
			}
		}
	} else if (mpegts_header.PID == MPEGTS_PID_SDT) {
		mpegts_psi_parse(&mpegts_psi, &msg[i+5]);
		// printf("SDT: "); psi_print(&psi);
	}

	if (mpegts_header.contains_payload) {
		if (mpegts_header.payload_unit_start_indicator) {
			if (mpegts_header.PID != app->video_PID_H264)
				goto cleanup;

			int pes_offset = i + 4;
			if (mpegts_header.adaption_field_control) {
				pes_offset = pes_offset + mpegts_adaption.adaptation_field_length + 1;
			}

			int pes_length = 0;
			pes_length = MPEGTS_PACKET_SIZE - pes_offset;

			PES pes = { 0 };
			pes_parse(&pes, &msg[pes_offset], app->offset, pes_offset, i);

			// printf("PES 0x%08llX | PTS: %" PRId64 " DTS: %" PRId64 "\n",
			// 	app->offset + pes_offset, pes.PTS, pes.DTS);
		} else {
			int es_offset = i + 4;
			if (mpegts_header.adaption_field_control) {
				es_offset = es_offset + mpegts_adaption.adaptation_field_length + 1;
			}
			int es_length = MPEGTS_PACKET_SIZE - es_offset;
			es_parse(&msg[es_offset], app->offset, es_offset, es_length);
		}

		// fwrite(payload, payload_length, 1, f_h264);
		// fflush(f_h264);
		// h264_len += payload_length;
	}

	// if (header.contains_payload) {
	// 	printf("PAYLOAD\n");
	// 	header_print(&header);
	// }
	// if (header.payload_unit_start_indicator) {
	// 	printf("PAYLOAD START\n");
	// 	header_print(&header);
	// }

cleanup:
	app->offset += MPEGTS_PACKET_SIZE;
}

void* udp_handler(void *args) {
	struct ip_mreq mreq;
	struct sockaddr_in addr;
	int i, addrlen, sock, msg_len;
	uint8_t msg[MSG_SIZE];

	App *app = (App*)args;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	bzero((char *)&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(EXAMPLE_PORT);
	addrlen = sizeof(addr);

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	mreq.imr_multiaddr.s_addr = inet_addr(EXAMPLE_GROUP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt mreq");
		exit(EXIT_FAILURE);
	}

	size_t written = 0;

	while (1) {
		memset(msg, 0, sizeof msg);
		msg_len = recvfrom(sock, msg, sizeof(msg), 0, (struct sockaddr *) &addr, &addrlen);
		if (msg_len < 0) {
			perror("recvfrom");
			exit(EXIT_FAILURE);
		} else if (msg_len == 0) {
			break;
		}

		for (i = 0; i < msg_len; i += MPEGTS_PACKET_SIZE)
			fifo_write(app->fifo, &msg[i], MPEGTS_PACKET_SIZE, &written);
	}
}

void* mpegts_handler(void *args) {
	int i = 0;
	size_t readed_len = 0;
	App *app = (App*)args;

	FILE *f_ts = fopen("./tmp/out.ts", "wb");

	for(;;) {
		fifo_wait_data(app->fifo);

		uint8_t msg[MPEGTS_PACKET_SIZE] = { 0 };
		fifo_read(app->fifo, msg, MPEGTS_PACKET_SIZE, &readed_len);

		// if (!readed_len) continue;

		// printf("readed_len %d\n", readed_len);
		// for (i = 0; i < MPEGTS_PACKET_SIZE; i++) {
		// 	printf("0x%02X ", msg[i]);
		// }
		// printf("\n");

		if (msg[0] == MPEGTS_SYNC_BYTE) {
			on_msg(app, msg);

			fwrite(msg, MPEGTS_PACKET_SIZE, 1, f_ts);
		}
	}
}

int main (int argc, char *argv[]) {
	int ret = EX_OK;

	Config *config = config_new();
	if (config == NULL) {
		fprintf(stderr, "failed to initialize config\n");
		exit(EXIT_FAILURE);
	}

	config_parse(config, argc, argv);
	config_print(config);

	if (config_validate(config)) { ret = EX_CONFIG; goto cleanup; }

	App *app = app_new();
	FIFO *fifo = fifo_new(30000*7*188);
	app->fifo = fifo;

	pthread_t udp_thread;
	pthread_create(&udp_thread, NULL, udp_handler, (void*)app);

	pthread_t mpegts_thread;
	pthread_create(&mpegts_thread, NULL, mpegts_handler, (void*)app);

	void *udp_thread_status;
	pthread_join(udp_thread, &udp_thread_status);

	void *mpegts_thread_status;
	pthread_join(mpegts_thread, &mpegts_thread_status);

cleanup:
	return ret;
}
