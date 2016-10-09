#ifndef __MPEGTS_MPEGTS__
#define __MPEGTS_MPEGTS__


#include <stdio.h>    // printf
#include <stdint.h>   // uint8_t
#include <inttypes.h> // PRIu64


typedef enum mpegts_table_id_enum MPEGTSTableID;
typedef enum mpegts_es_type_enum MPEGTSESType;

// ETSI EN 300 468 V1.3.1 (1998-02)
// ETSI EN 300 468 V1.11.1 (2010-04)
enum mpegts_table_id_enum {
	MPEGTS_TABLE_ID_PROGRAM_ASSOCIATION_SECTION                         = 0x00,
	MPEGTS_TABLE_ID_CONDITIONAL_ACCESS_SECTION                          = 0x01,
	MPEGTS_TABLE_ID_PROGRAM_MAP_SECTION                                 = 0x02,
	MPEGTS_TABLE_ID_TRANSPORT_STREAM_DESCRIPTION_SECTION                = 0x03,
	MPEGTS_TABLE_ID_NETWORK_INFORMATION_SECTION_ACTUAL_NETWORK          = 0x40,
	MPEGTS_TABLE_ID_NETWORK_INFORMATION_SECTION_OTHER_NETWORK           = 0x41,
	MPEGTS_TABLE_ID_SERVICE_DESCRIPTION_SECTION_ACTUAL_TRANSPORT_STREAM = 0x42,
	MPEGTS_TABLE_ID_SERVICE_DESCRIPTION_SECTION_OTHER_TRANSPORT_STREAM  = 0x46,
	MPEGTS_TABLE_ID_BOUQUET_ASSOCIATION_SECTION                         = 0x4A,
	MPEGTS_TABLE_ID_EVENT_INFORMATION_SECTION_ACTUAL_TRANSPORT_STREAM   = 0x4E,
	MPEGTS_TABLE_ID_EVENT_INFORMATION_SECTION_OTHER_TRANSPORT_STREAM    = 0x4F,
	MPEGTS_TABLE_ID_TIME_DATE_SECTION                                   = 0x70,
	MPEGTS_TABLE_ID_RUNNING_STATUS_SECTION                              = 0x71,
	MPEGTS_TABLE_ID_STUFFING_SECTION                                    = 0x72,
	MPEGTS_TABLE_ID_TIME_OFFSET_SECTION                                 = 0x73,
	MPEGTS_TABLE_ID_APPLICATION_INFORMATION_SECTION                     = 0x74,
	MPEGTS_TABLE_ID_CONTAINER_SECTION                                   = 0x75,
	MPEGTS_TABLE_ID_RELATED_CONTENT_SECTION                             = 0x76,
	MPEGTS_TABLE_ID_CONTENT_IDENTIFIER_SECTION                          = 0x77,
	MPEGTS_TABLE_ID_MPE_FEC_SECTION                                     = 0x78,
	MPEGTS_TABLE_ID_RESOLUTION_NOTIFICATION_SECTION                     = 0x79,
	MPEGTS_TABLE_ID_MPE_IFEC_SECTION                                    = 0x7A,
	MPEGTS_TABLE_ID_DISCONTINUITY_INFORMATION_SECTION                   = 0x7E,
	MPEGTS_TABLE_ID_SELECTION_INFORMATION_SECTION                       = 0x7F
};

// Each elementary stream in a transport stream
// is identified by an 8-bit elementary stream
// type assignment.
enum mpegts_es_type_enum {
	MPEGTS_STREAM_TYPE_RESERVED                = 0x00,
	MPEGTS_STREAM_TYPE_VIDEO_MPEG1             = 0x01,
	MPEGTS_STREAM_TYPE_VIDEO_MPEG2             = 0x02,
	MPEGTS_STREAM_TYPE_AUDIO_MPEG1             = 0x03,
	MPEGTS_STREAM_TYPE_AUDIO_MPEG2             = 0x04,
	MPEGTS_STREAM_TYPE_PRIVATE_SECTIONS        = 0x05,
	MPEGTS_STREAM_TYPE_PRIVATE_PES_PACKETS     = 0x06,
	MPEGTS_STREAM_TYPE_MHEG                    = 0x07,
	MPEGTS_STREAM_TYPE_H222_DSM_CC             = 0x08, // Digital storage media command and control (DSM-CC)
	MPEGTS_STREAM_TYPE_H222_DSM_CC_AUX         = 0x09,
	MPEGTS_STREAM_TYPE_AUDIO_AAC_ADTS          = 0x0F,
	MPEGTS_STREAM_TYPE_VIDEO_MPEG4_H263        = 0x10,
	MPEGTS_STREAM_TYPE_AUDIO_MPEG4_LOAS        = 0x11,
	MPEGTS_STREAM_TYPE_VIDEO_MPEG4_FLEX_MUX    = 0x12,
	MPEGTS_STREAM_TYPE_VIDEO_H264              = 0x1B,
	MPEGTS_STREAM_TYPE_VIDEO_H265              = 0x24,
	MPEGTS_STREAM_TYPE_AUIDO_AAC_AES_128_CBC   = 0xCF,
};

#define MPEGTS_STREAM_TYPE_RESERVED_STR "Reserved"
#define MPEGTS_STREAM_TYPE_VIDEO_MPEG1_STR "ISO/IEC 11172-2 (MPEG-1 video)" \
                                           " in a packetized stream"
#define MPEGTS_STREAM_TYPE_VIDEO_MPEG2_STR "ITU-T Rec. H.262 and ISO/IEC 13818-2" \
                                           " (MPEG-2 higher rate interlaced video)" \
                                           " in a packetized stream"
#define MPEGTS_STREAM_TYPE_AUDIO_MPEG1_STR "ISO/IEC 11172-3 (MPEG-1 audio)" \
                                           " in a packetized stream"
#define MPEGTS_STREAM_TYPE_AUDIO_MPEG2_STR "ISO/IEC 13818-3 (MPEG-2 halved sample rate audio)" \
                                           " in a packetized stream"
#define MPEGTS_STREAM_TYPE_PRIVATE_SECTIONS_STR "ITU-T Rec. H.222 and ISO/IEC 13818-1" \
                                                " (MPEG-2 tabled data)" \
                                                " privately defined"
#define MPEGTS_STREAM_TYPE_PRIVATE_PES_PACKETS_STR "ITU-T Rec. H.222 and ISO/IEC 13818-1" \
                                                   " (MPEG-2 packetized data)" \
                                                   " privately defined (i.e., DVB subtitles/VBI and AC-3)"
#define MPEGTS_STREAM_TYPE_MHEG_STR "ISO/IEC 13522 (MHEG)" \
                                    " in a packetized stream"
#define MPEGTS_STREAM_TYPE_H222_DSM_CC_STR "ITU-T Rec. H.222 and ISO/IEC 13818-1 DSM CC" \
                                      " in a packetized stream"
#define MPEGTS_STREAM_TYPE_AUDIO_AAC_ADTS_STR "ISO/IEC 13818-7 ADTS AAC" \
                                              " (MPEG-2 lower bit-rate audio)" \
                                              " in a packetized stream"
#define MPEGTS_STREAM_TYPE_VIDEO_MPEG4_H263_STR "ISO/IEC 14496-2 (MPEG-4 H.263 based video)" \
                                                " in a packetized stream"
#define MPEGTS_STREAM_TYPE_VIDEO_MPEG4_FLEX_MUX_STR "ISO/IEC 14496-1 (MPEG-4 FlexMux)" \
                                                    " in a packetized stream"
#define MPEGTS_STREAM_TYPE_VIDEO_H264_STR "ITU-T Rec. H.264 and ISO/IEC 14496-10" \
                                          " (lower bit-rate video) in a" \
                                          " packetized stream"
#define MPEGTS_STREAM_TYPE_VIDEO_H265_STR "ITU-T Rec. H.265 and ISO/IEC 23008-2 " \
                                          "(Ultra HD video) in a packetized stream"
#define MPEGTS_STREAM_TYPE_AUIDO_AAC_AES_128_CBC_STR "ISO/IEC 13818-7 ADTS AAC" \
                                                     " with AES-128-CBC frame encryption" \
                                                     " in a packetized stream"


/* header.c */
typedef struct mpegts_header_s MPEGTSHeader;

struct mpegts_header_s {
	uint8_t
		transcport_error_indicator:1,   // TEI
		payload_unit_start_indicator:1, // PUSI
		                                // Set when a PES, PSI, or DVB-MIP
		                                // packet begins immediately following the header.
		transcport_priority:1;
	uint16_t PID:13;
	uint8_t
		transport_scrambling_control:2, // TSC
		adaption_field_control:1,
		contains_payload:1,
		continuity_counter:4;
};

void mpegts_header_parse(MPEGTSHeader *it, uint8_t *data);
void mpegts_header_print_json(MPEGTSHeader *it);


/* adaption.c */
typedef struct mpegts_adaption_s MPEGTSAdaption;
typedef struct mpegts_PCR_s MPEGTSPCR;

// Program Clock Reference
struct mpegts_PCR_s {
	uint64_t base:33;
	uint16_t ext:9;
};

struct mpegts_adaption_s {
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

	MPEGTSPCR pcr;
};

void mpegts_adaption_parse(MPEGTSAdaption *it, uint8_t *data);
void mpegts_adaption_print_json(MPEGTSAdaption *it);

void mpegts_pcr_parse(MPEGTSPCR *it, uint8_t *data);
void mpegts_pcr_print_json(MPEGTSPCR *it);


/* psi.c */
typedef struct mpegts_PSI_s MPEGTSPSI;
typedef struct mpegts_PAT_s MPEGTSPAT;
typedef struct mpegts_PMT_s MPEGTSPMT;
typedef struct mpegts_NIT_s MPEGTSNIT;

// Program Specific Information
struct mpegts_PSI_s {
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
};

// PSI -> PAT
// Program association specific data
struct mpegts_PAT_s {
	uint16_t program_number;  // Relates to the Table ID extension in the associated PMT.
	                          // A value of 0 is reserved for a NIT packet identifier.
	uint16_t program_map_PID; // The packet identifier that contains the associated PMT
};

// PSI -> PMT
// Program map specific data
struct mpegts_PMT_s { };

// PSI -> CAT
// Conditional access specific data
struct mpegts_CAT_s { };

// PSI -> NIT
// network information specific data
struct mpegts_NIT_s { };

void mpegts_psi_parse(MPEGTSPSI *it, uint8_t *data);
void mpegts_psi_print_json(MPEGTSPSI *it);

void mpegts_pat_parse(MPEGTSPAT *it, uint8_t *data);
void mpegts_pat_print_json(MPEGTSPAT *it);


/* es.c */
const char* mpegts_es_type_string(MPEGTSESType it);


#endif // __MPEGTS_MPEGTS__
