#include <stdio.h>
#include <stdlib.h>     // EXIT_SUCCESS, EXIT_FAILURE, malloc
#include <inttypes.h>
#include <unistd.h>     // close
#include <fcntl.h>      // open
#include <string.h>     // memcpy, memset, size_t
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>

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

// ETSI EN 300 468 V1.3.1 (1998-02)
// ETSI EN 300 468 V1.11.1 (2010-04)
#define MPEGTS_TABLE_ID_PROGRAM_ASSOCIATION_SECTION                          0x00
#define MPEGTS_TABLE_ID_CONDITIONAL_ACCESS_SECTION                           0x01
#define MPEGTS_TABLE_ID_PROGRAM_MAP_SECTION                                  0x02
#define MPEGTS_TABLE_ID_TRANSPORT_STREAM_DESCRIPTION_SECTION                 0x03
#define MPEGTS_TABLE_ID_NETWORK_INFORMATION_SECTION_ACTUAL_NETWORK           0x40
#define MPEGTS_TABLE_ID_NETWORK_INFORMATION_SECTION_OTHER_NETWORK            0x41
#define MPEGTS_TABLE_ID_SERVICE_DESCRIPTION_SECTION_ACTUAL_TRANSPORT_STREAM  0x42
#define MPEGTS_TABLE_ID_SERVICE_DESCRIPTION_SECTION_OTHER_TRANSPORT_STREAM   0x46
#define MPEGTS_TABLE_ID_BOUQUET_ASSOCIATION_SECTION                          0x4A
#define MPEGTS_TABLE_ID_EVENT_INFORMATION_SECTION_ACTUAL_TRANSPORT_STREAM    0x4E
#define MPEGTS_TABLE_ID_EVENT_INFORMATION_SECTION_OTHER_TRANSPORT_STREAM     0x4F
#define MPEGTS_TABLE_ID_TIME_DATE_SECTION                                    0x70
#define MPEGTS_TABLE_ID_RUNNING_STATUS_SECTION                               0x71
#define MPEGTS_TABLE_ID_STUFFING_SECTION                                     0x72
#define MPEGTS_TABLE_ID_TIME_OFFSET_SECTION                                  0x73
#define MPEGTS_TABLE_ID_APPLICATION_INFORMATION_SECTION                      0x74
#define MPEGTS_TABLE_ID_CONTAINER_SECTION                                    0x75
#define MPEGTS_TABLE_ID_RELATED_CONTENT_SECTION                              0x76
#define MPEGTS_TABLE_ID_CONTENT_IDENTIFIER_SECTION                           0x77
#define MPEGTS_TABLE_ID_MPE_FEC_SECTION                                      0x78
#define MPEGTS_TABLE_ID_RESOLUTION_NOTIFICATION_SECTION                      0x79
#define MPEGTS_TABLE_ID_MPE_IFEC_SECTION                                     0x7A
#define MPEGTS_TABLE_ID_DISCONTINUITY_INFORMATION_SECTION                    0x7E
#define MPEGTS_TABLE_ID_SELECTION_INFORMATION_SECTION                        0x7F

#define MPEGTS_STREAM_TYPE_VIDEO_MPEG1         0x01
#define MPEGTS_STREAM_TYPE_VIDEO_MPEG2         0x02
#define MPEGTS_STREAM_TYPE_AUDIO_MPEG1         0x03
#define MPEGTS_STREAM_TYPE_AUDIO_MPEG2         0x04
#define MPEGTS_STREAM_TYPE_PRIVATE_SECTIONS    0x05
#define MPEGTS_STREAM_TYPE_PRIVATE_PES_PACKETS 0x06
#define MPEGTS_STREAM_TYPE_MHEG                0x07
#define MPEGTS_STREAM_TYPE_AUDIO_AAC_ADTS      0x0F
#define MPEGTS_STREAM_TYPE_VIDEO_MPEG4         0x10
#define MPEGTS_STREAM_TYPE_VIDEO_AAC_LATM      0x11
#define MPEGTS_STREAM_TYPE_VIDEO_H264          0x1B
#define MPEGTS_STREAM_TYPE_VIDEO_H265          0x24

static void mpegts_stream_type_str(uint8_t v, char *out) {
	switch(v) {
	case MPEGTS_STREAM_TYPE_VIDEO_MPEG1:
		strcpy(out, "ISO/IEC 11172-2 (MPEG-1 video) "
			          "in a packetized stream");
		break;
	case MPEGTS_STREAM_TYPE_VIDEO_MPEG2:
		strcpy(out, "ITU-T Rec. H.262 and ISO/IEC 13818-2 "
			          "(MPEG-2 higher rate interlaced video) "
			          "in a packetized stream");
		break;
	case MPEGTS_STREAM_TYPE_AUDIO_MPEG1:
		strcpy(out, "ISO/IEC 11172-3 (MPEG-1 audio) "
			          "in a packetized stream");
		break;
	case MPEGTS_STREAM_TYPE_AUDIO_MPEG2:
		break;
	case MPEGTS_STREAM_TYPE_PRIVATE_SECTIONS:
		break;
	case MPEGTS_STREAM_TYPE_PRIVATE_PES_PACKETS:
		break;
	case MPEGTS_STREAM_TYPE_MHEG:
		break;
	case MPEGTS_STREAM_TYPE_AUDIO_AAC_ADTS:
		break;
	case MPEGTS_STREAM_TYPE_VIDEO_MPEG4:
		break;
	case MPEGTS_STREAM_TYPE_VIDEO_AAC_LATM:
		break;
	case MPEGTS_STREAM_TYPE_VIDEO_H264:
		strcpy(out, "ITU-T Rec. H.264 and ISO/IEC 14496-10 "
			          "(lower bit-rate video) in a "
			          "packetized stream");
		break;
	case MPEGTS_STREAM_TYPE_VIDEO_H265:
		strcpy(out, "ITU-T Rec. H.265 and ISO/IEC 23008-2 "
			          "(Ultra HD video) in a packetized stream");
		break;
	}
}

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
	FIFO *fifo;
	uint16_t program_map_PID;
	uint16_t video_PID_H264;
} App;

static App *app_new(void) {
	App *it = (App*)calloc(1, sizeof(App));
	it->program_map_PID = 0;
	it->video_PID_H264 = 0;
}

typedef struct Header {
	uint8_t
		transcport_error_indicator:1,
		payload_unit_start_indicator:1,
		transcport_priority:1;
	uint16_t PID:13;
	uint8_t
		transport_scrambling_control:2,
		adaption_field_control:1,
		contains_payload:1,
		continuity_counter:4;
} Header;

static void header_parse(Header *it, uint8_t *data) {
	uint8_t transcport_error_indicator = 0;
	uint8_t payload_unit_start_indicator = 0;
	uint8_t transcport_priority = 0;
	uint16_t PID = 0;
	uint8_t transport_scrambling_control = 0;
	uint8_t adaption_field_control = 0;
	uint8_t contains_payload = 0;
	uint8_t continuity_counter = 0;

	transcport_error_indicator = !!( (uint8_t)data[0] & 0x80 );
	payload_unit_start_indicator = !!( (uint8_t)data[0] & 0x40 );
	transcport_priority = !!( (uint8_t)data[0] & 0x20 );
	PID = ( (uint16_t)data[0] << 8 | (uint16_t)data[1] ) & 0x1fff;
	transport_scrambling_control = ((uint8_t)data[2] & 0xc0) >> 6;
	adaption_field_control = !!( (uint8_t)data[2] & 0x20 );
	contains_payload = !!( (uint8_t)data[2] & 0x10 );
	continuity_counter = (uint8_t)data[2] & 0xf;

	it->transcport_error_indicator = transcport_error_indicator;
	it->payload_unit_start_indicator = payload_unit_start_indicator;
	it->transcport_priority = transcport_priority;
	it->PID = PID;
	it->transport_scrambling_control = transport_scrambling_control;
	it->adaption_field_control = adaption_field_control;
	it->contains_payload = contains_payload;
	it->continuity_counter = continuity_counter;
}

static void header_print(Header *it) {
	printf(
		"{\"TEI\": %d"
		", \"PUSI\": %d"
		", \"TP\": %d"
		", \"PID\": 0x%04x"
		", \"TSC\": 0x%02x"
		", \"adaption\": %d"
		", \"payload\": %d"
		", \"continuity-counter\": %d"
		"}\n",
		it->transcport_error_indicator,
		it->payload_unit_start_indicator,
		it->transcport_priority,
		it->PID,
		it->transport_scrambling_control,
		it->adaption_field_control,
		it->contains_payload,
		it->continuity_counter
	);
}

typedef struct Adaption {
	uint8_t
		adaptation_field_length:8,
		discontinuity_indicator:1,
		random_access_indicator:1,
		elementary_stream_priority_indicator:1,
		PCR_flag:1,
		OPCR_flag:1,
		splicing_point_flag:1,
		transport_private_data_flag:1,
		adaptation_field_extension_flag:1;
} Adaption;

static void adaption_parse(Adaption *it, uint8_t *data) {
	uint8_t adaptation_field_length	= (uint8_t)data[0];
	uint8_t discontinuity_indicator = !!( (uint8_t)data[1] & 0x80 );
	uint8_t random_access_indicator = !!( (uint8_t)data[1] & 0x40 );
	uint8_t elementary_stream_priority_indicator = !!( (uint8_t)data[1] & 0x20 );
	uint8_t PCR_flag = !!( (uint8_t)data[1] & 0x10 );
	uint8_t OPCR_flag = !!( (uint8_t)data[1] & 0x08 );
	uint8_t splicing_point_flag = !!( (uint8_t)data[1] & 0x04 );
	uint8_t transport_private_data_flag = !!( (uint8_t)data[1] & 0x02 );
	uint8_t adaptation_field_extension_flag = !!( (uint8_t)data[1] & 0x01 );

	it->adaptation_field_length = adaptation_field_length;
	it->discontinuity_indicator = discontinuity_indicator;
	it->random_access_indicator = random_access_indicator;
	it->elementary_stream_priority_indicator = elementary_stream_priority_indicator;
	it->PCR_flag = PCR_flag;
	it->OPCR_flag = OPCR_flag;
	it->splicing_point_flag = splicing_point_flag;
	it->transport_private_data_flag = transport_private_data_flag;
	it->adaptation_field_extension_flag = adaptation_field_extension_flag;
}

static void adaption_print(Adaption *it) {
	printf(
		"{\"adaptation-field-length\": %d"
		", \"discontinuity-indicator\": %d"
		", \"random-access-indicator\": %d"
		", \"elementary-stream-priority-indicator\": %d"
		", \"PCR-flag\": %d"
		", \"OPCR-flag\": %d"
		", \"splicing-point-flag\": %d"
		", \"transport-private-data-flag\": %d"
		", \"adaptation-field-extension-flag\": %d"
		"}\n",
		it->adaptation_field_length,
		it->discontinuity_indicator,
		it->random_access_indicator,
		it->elementary_stream_priority_indicator,
		it->PCR_flag,
		it->OPCR_flag,
		it->splicing_point_flag,
		it->transport_private_data_flag,
		it->adaptation_field_extension_flag
	);
};

// Program Clock Reference
typedef struct PCR {
	uint64_t base:33;
	uint16_t ext:9;
} PCR;

static void pcr_parse(PCR *it, uint8_t *data) {
	uint64_t base = 0;
	uint16_t ext = 0;

	base = ((
		((uint64_t)data[0]  & 0xFF) << 32 |
		((uint64_t)data[1]  & 0xFF) << 24 |
		((uint64_t)data[2]  & 0xFF) << 16 |
		((uint64_t)data[3]  & 0xFF) << 8  |
		((uint64_t)data[4] & 0x80)
	) >> 7);

	ext = (
			((uint16_t)data[4] & 0x01) << 8 |
			((uint16_t)data[5] & 0xFF)
	);

	it->base = base;
	it->ext = ext;
}

static void pcr_print(PCR *it) {
	printf("{\"base\": %"PRIu64", \"ext\": %d}\n",
		it->base,
		it->ext
	);
}

// Program Specific Information
typedef struct PSI {
	// header
	uint8_t
		table_id:8,
		section_syntax_indicator:1,
		private_bit:1,
		reserved_bits:2,
		section_length_unused_bits:2;
	uint16_t section_length:10;

	// table syntax section
	uint16_t transport_stream_id:16;
	uint8_t
		version_number:5,
		curent_next_indicator:1,
		section_number:8,
		last_section_number:8;

	uint32_t CRC32:32;
} PSI;

static void psi_parse(PSI *it, uint8_t *data) {
	// PSI - header
	uint8_t table_id = (uint8_t)data[0];
	uint8_t section_syntax_indicator = !!( (uint8_t)data[1] & 0x80 );
	uint8_t private_bit = !!( (uint8_t)data[1] & 0x40 );
	uint8_t reserved_bits = (uint8_t)data[1] & 0x30;
	uint8_t section_length_unused_bits = (uint8_t)data[1] & 0x0C;
	uint16_t section_length = (((uint16_t)data[1] & 0x03 ) << 8) | ((uint16_t)data[2] & 0xFF);

	// PSI - table syntax section
	uint16_t transport_stream_id = (
		(((uint16_t)data[3] & 0xFF) << 8) |
		((uint16_t)data[4] & 0xFF)
	);
	uint8_t version_number = (uint8_t)data[5] & 0x3E;
	uint8_t curent_next_indicator = (uint8_t)data[5] & 0x01;
	uint8_t section_number = (uint8_t)data[6];
	uint8_t last_section_number = (uint8_t)data[7];

	uint16_t CRC32_i = 2 + section_length;
	uint32_t CRC32 = (
		(((uint32_t)data[CRC32_i-3] & 0xFF) << 24) |
		(((uint32_t)data[CRC32_i-2] & 0xFF) << 16) |
		(((uint32_t)data[CRC32_i-1] & 0xFF) << 8) |
		((uint32_t)data[CRC32_i] & 0xFF)
	);

	it->table_id = table_id;
	it->section_syntax_indicator = section_syntax_indicator;
	it->private_bit = private_bit;
	it->reserved_bits = reserved_bits;
	it->section_length_unused_bits = section_length_unused_bits;
	it->section_length = section_length;

	it->transport_stream_id = transport_stream_id;
	it->version_number = version_number;
	it->curent_next_indicator = curent_next_indicator;
	it->section_number = section_number;
	it->last_section_number = last_section_number;

	it->CRC32 = CRC32;
}

static void psi_print(PSI *it) {
	// printf("0x%02X | %d | %d | %d | %d | %d | %d | %d | %d | 0x%04x\n", table_id, section_syntax_indicator, private_bit, section_length, transport_stream_id, version_number, curent_next_indicator, section_number, last_section_number, CRC32);
	printf(
		"{\"table-id\": %d (0x%02x)"
		", \"section-syntax-indicator\": %d"
		", \"private-bit\": %d"
		", \"section-length\": %d"
		", \"transport-stream-id\": %d"
		", \"version-number\": %d"
		", \"curent-next-indicator\": %d"
		", \"section-number\": %d"
		", \"last-section-number\": %d"
		", \"CRC32\": 0x%08X"
		"}\n",
		it->table_id, it->table_id,
		it->section_syntax_indicator,
		it->private_bit,
		it->section_length,
		it->transport_stream_id,
		it->version_number,
		it->curent_next_indicator,
		it->section_number,
		it->last_section_number,
		it->CRC32
	);
}

void on_msg(App *app, uint8_t *msg) {
	if (msg[0] != MPEGTS_SYNC_BYTE) return;

	int i = 0;
	Header header = { 0 };
	header_parse(&header, &msg[i+1]);

	Adaption adaption = { 0 };
	if (header.adaption_field_control) {
		adaption_parse(&adaption, &msg[i+4]);

		if (adaption.PCR_flag) {
			PCR pcr = { 0	};
			pcr_parse(&pcr, &msg[i+6]);

			// header_print(&header);
			// adaption_print(&adaption);
			// pcr_print(&pcr);
		}
	}

	if (header.PID == MPEGTS_PID_PAT) {
		PSI psi = { 0 };
		psi_parse(&psi, &msg[i+5]);
		// printf("PAT: "); psi_print(&psi);

		if (psi.table_id == MPEGTS_TABLE_ID_PROGRAM_ASSOCIATION_SECTION) {
			// PAT (Program association specific data)
			uint16_t program_number = (((uint16_t)msg[i+13] & 0xFF) << 8) | ((uint16_t)msg[i+14] & 0xFF);
			app->program_map_PID = (((uint16_t)msg[i+15] & 0x1F) << 8) | ((uint16_t)msg[i+16] & 0xFF);
			// printf("%d 0x%04x %d\n", program_number, app->program_map_PID, app->program_map_PID);
		}
	} else if ((app->program_map_PID) && (header.PID == app->program_map_PID)) {
		PSI psi = { 0 };
		psi_parse(&psi, &msg[i+5]);
		// printf("PMT: "); psi_print(&psi);

		if (psi.table_id == MPEGTS_TABLE_ID_PROGRAM_MAP_SECTION) {
			int16_t section_length_unreaded = (int16_t)psi.section_length;
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

				char stream_type_name[255] = { 0 };
				mpegts_stream_type_str(stream_type, stream_type_name);
				// printf("\t - 0x%02x | 0x%02x | %d | %d | %s\n", stream_type, elementary_PID, ES_info_length, psi.section_length, stream_type_name);

				int ES_info_start = pmt_start+pmt_offset+5;
				int ES_info_offset = 0;
				int16_t ES_info_length_unreaded = ES_info_length;
				while (ES_info_length_unreaded > 0) {
					uint8_t descriptor_tag = (uint8_t)msg[ES_info_start+ES_info_offset];
					uint8_t descriptor_length = (uint8_t)msg[ES_info_start+ES_info_offset+1];
					// printf("\t\t - 0x%02x | %d\n", descriptor_tag, descriptor_length);
					ES_info_length_unreaded -= (2 + descriptor_length);
					descriptor_length += (2 + descriptor_length);
				}

				section_length_unreaded -= (5 + ES_info_length);
				pmt_offset +=	(5 + ES_info_length);

				if (stream_type == MPEGTS_STREAM_TYPE_VIDEO_H264)
					app->video_PID_H264 = elementary_PID;
			}
		}
	} else if (header.PID == MPEGTS_PID_SDT) {
		PSI psi = { 0 };
		psi_parse(&psi, &msg[i+5]);
		// printf("SDT: "); psi_print(&psi);
	}

	if ((header.contains_payload) && (header.payload_unit_start_indicator)) {
		if (header.PID != app->video_PID_H264)
			return;

		int pes_offset = i + 4;
		if (header.adaption_field_control) {
			pes_offset = pes_offset + adaption.adaptation_field_length + 1;
		}

		uint8_t *pes = &msg[pes_offset];
		int pes_length = 0;
		pes_length = MPEGTS_PACKET_SIZE - pes_offset;

		uint32_t start_code = (
			(uint32_t)pes[0] & 0xFF << 16 |
			(uint32_t)pes[1] & 0xFF << 8 |
			(uint32_t)pes[2] & 0xFF
		);
		// http://dvd.sourceforge.net/dvdinfo/pes-hdr.html
		if (start_code == PES_START_CODE) {
			uint8_t stream_id = (uint8_t)pes[3];
			uint16_t pes_packet_length = (
				(uint16_t)pes[4] & 0xFF << 8 |
				(uint16_t)pes[5] & 0xFF
			);

			// PES header
			// 10 binary or 0x2 hex
			uint8_t marker_bits = (((uint8_t)pes[6] & 0xC0) >> 6);
			// 00 - not scrambled
			uint8_t scrambling_control = (uint8_t)pes[6] & 0x30;
			uint8_t priority = !!( (uint8_t)pes[6] & 0x08 );
			// 1 indicates that the PES packet
			// header is immediately followed by
			// the video start code or audio syncword
			uint8_t data_alignment_indicator = !!( (uint8_t)pes[6] & 0x04 );
			uint8_t copyright = !!( (uint8_t)pes[6] & 0x02 );
			uint8_t original_or_copy = !!( (uint8_t)pes[6] & 0x01 );
			// 11 = both present;
			// 01 is forbidden;
			// 10 = only PTS;
			// 00 = no PTS or DTS
			uint8_t PTS_DTS_indicator = (((uint8_t)pes[7] & 0xC0) >> 6);
			// This is the Elementary Stream Clock Reference,
			// used if the stream and system levels are not synchronized'
			// (i.e. ESCR differs from SCR in the PACK header).
			uint8_t ESCR_flag = (uint8_t)pes[7] & 0x20;
			// The rate at which data is delivered for this stream,
			// in units of 50 bytes/second.
			uint8_t ES_rate_flag = (uint8_t)pes[7] & 0x10;
			uint8_t DSM_trick_mode_flag = (uint8_t)pes[7] & 0x08;
			uint8_t additional_copy_info_flag = (uint8_t)pes[7] & 0x04;
			uint8_t CRC_flag = (uint8_t)pes[7] & 0x02;
			uint8_t extension_flag = (uint8_t)pes[7] & 0x01;
			uint8_t PES_header_length = (uint8_t)pes[8];
			// printf("%d | %d | 0x%02x\n", data_alignment_indicator, PES_header_length, PTS_DTS_indicator);
			if ((PTS_DTS_indicator == PES_PTS_DTS_INDICATOR_PTS) ||
				  (PTS_DTS_indicator == PES_PTS_DTS_INDICATOR_PTS_DTS)) {
				uint64_t PTS = (
					(((uint64_t)pes[9]  & 0x0E) << 32) |
					(((uint64_t)pes[10] & 0xFF) << 24) |
					(((uint64_t)pes[11] & 0xFE) << 16) |
					(((uint64_t)pes[12] & 0xFF) << 8) |
					 ((uint64_t)pes[13] & 0xFE)
				);
				uint64_t DTS = 0;
				if (PTS_DTS_indicator == PES_PTS_DTS_INDICATOR_PTS_DTS) {
					DTS = (
						(((uint64_t)pes[14]  & 0x0E) << 32) |
						(((uint64_t)pes[15] & 0xFF) << 24) |
						(((uint64_t)pes[16] & 0xFE) << 16) |
						(((uint64_t)pes[17] & 0xFF) << 8) |
						 ((uint64_t)pes[18] & 0xFE)
					);
				}

				printf("PTS: %d DTS: %d\n", PTS, DTS);
			}

			// ES
			uint8_t *es = &pes[9+PES_header_length];
			int es_offset = pes_offset + PES_header_length + 9;
			int es_length = MPEGTS_PACKET_SIZE - (es_offset - i);
			int es_i = 0;

			for (es_i = 0; es_i < es_length; es_i++) {
				uint32_t es_start_code = 0;
				uint8_t got_es_start_code = 0;

				es_start_code = (
					(uint32_t)es[es_i] & 0xFF << 24 |
					(uint32_t)es[es_i+1] & 0xFF << 16 |
					(uint32_t)es[es_i+2] & 0xFF << 8  |
					(uint32_t)es[es_i+3] & 0xFF
				);
				if (es_start_code == ES_START_CODE_LONG) {
					es_i += 4;
					got_es_start_code = 1;
				} else {
					es_start_code = (
						(uint32_t)es[es_i] & 0xFF << 16 |
						(uint32_t)es[es_i+1] & 0xFF << 8 |
						(uint32_t)es[es_i+2] & 0xFF
					);
					if (es_start_code == ES_START_CODE_SHORT) {
						es_i += 3;
						got_es_start_code = 1;
					}
				}

				if (got_es_start_code) {
					uint8_t forbidden_zero_bit = es[es_i] & 0x80;
					if (forbidden_zero_bit != 0) continue;

					uint8_t nal_ref_idc = es[es_i] & 0x60;
					uint8_t nal_type = es[es_i] & 0x1F;

					char nal_type_name[255] = { 0 };
					nal_type_str(nal_type, nal_type_name);

					if (
						(nal_type == NAL_TYPE_AUD) ||
						(nal_type == NAL_TYPE_SEI) ||
						(nal_type == NAL_TYPE_SLICE) ||
						(nal_type == NAL_TYPE_SPS) ||
						(nal_type == NAL_TYPE_PPS)
					) {
						switch (nal_type) {
						case NAL_TYPE_AUD:
							printf(COLOR_BRIGHT_YELLOW "%s" COLOR_RESET "\n", nal_type_name);
							break;
						case NAL_TYPE_SEI:
							printf(COLOR_BRIGHT_BLUE "%s" COLOR_RESET "\n", nal_type_name);
							break;
						case NAL_TYPE_SPS:
							printf(COLOR_BRIGHT_WHITE "%s" COLOR_RESET "\n", nal_type_name);
							break;
						case NAL_TYPE_PPS:
							printf(COLOR_BRIGHT_WHITE "%s" COLOR_RESET "\n", nal_type_name);
							break;
						}

						// slice hader
						if (nal_type == NAL_TYPE_SLICE) {
							uint32_t offset = 0;
							uint32_t first_mb_in_slice = decode_u_golomb(&es[es_i+1], &offset);
							uint32_t slice_type = decode_u_golomb(&es[es_i+1], &offset);
							uint32_t pic_parameter_set_id = decode_u_golomb(&es[es_i+1], &offset);
							uint32_t frame_num = decode_u_golomb(&es[es_i+1], &offset);
							uint32_t pic_order_cnt_lsb = decode_u_golomb(&es[es_i+1], &offset);

							switch (slice_type) {
							case 0:
							case 5:
								printf(COLOR_BRIGHT_CYAN "H264 P slice #%d" COLOR_RESET "\n", frame_num);
								break;
							case 1:
							case 6:
								printf(COLOR_BRIGHT_GREEN "H264 B slice #%d" COLOR_RESET "\n", frame_num);
								break;
							case 2:
							case 7:
								printf(COLOR_BRIGHT_RED "H264 I slice #%d" COLOR_RESET "\n", frame_num);
								break;
							}
						}
					}
				}
			}
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
}

void udp_handler(void *args) {
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

	while (1) {
		memset(msg, 0, sizeof msg);
		msg_len = recvfrom(sock, msg, sizeof(msg), 0, (struct sockaddr *) &addr, &addrlen);
		if (msg_len < 0) {
			perror("recvfrom");
			exit(EXIT_FAILURE);
		} else if (msg_len == 0) {
			break;
		}

		for (i = 0; i < msg_len; i += MPEGTS_PACKET_SIZE) {
			fifo_write(app->fifo, &msg[i], MPEGTS_PACKET_SIZE);
		}
	}
}

void mpegts_handler(void *args) {
	int i = 0;
	size_t readed_len = 0;
	App *app = (App*)args;

	FILE *f_ts = fopen("./tmp/out.ts", "ab");

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
	Config *config = config_new();
	if (config == NULL) {
		fprintf(stderr, "failed to initialize config\n");
		exit(EXIT_FAILURE);
	}

	config_parse(config, argc, argv);
	config_print(config);

	url_parse(config->i);

	exit(EXIT_SUCCESS);

	App *app = app_new();
	FIFO *fifo = fifo_new(100*7*188);
	app->fifo = fifo;

	pthread_t udp_thread;
	pthread_create(&udp_thread, NULL, udp_handler, (void*)app);

	pthread_t mpegts_thread;
	pthread_create(&mpegts_thread, NULL, mpegts_handler, (void*)app);

	void *udp_thread_status;
	pthread_join(udp_thread, &udp_thread_status);

	void *mpegts_thread_status;
	pthread_join(mpegts_thread, &mpegts_thread_status);

	exit(EXIT_SUCCESS);
}
