#ifndef __MPEGTS_MPEGTS__
#define __MPEGTS_MPEGTS__


#include <stdio.h>    // printf
#include <stdint.h>   // uint8_t
#include <string.h>   // memcpy
#include <stdlib.h>   // calloc, realloc
#include <inttypes.h> // PRIu64


#define MPEGTS_SYNC_BYTE    0x47
#define MPEGTS_PACKET_COUNT 7
#define MPEGTS_PACKET_SIZE  188

#define MPEGTS_PID_PAT      0x0000
#define MPEGTS_PID_CAT      0x0001
#define MPEGTS_PID_TSDT     0x0002
#define MPEGTS_PID_CIT      0x0003
#define MPEGTS_PID_SDT      0x0011
#define MPEGTS_PID_NULL     0x1FFF


typedef enum mpegts_table_id_enum MPEGTSTableID;

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
	MPEGTS_TABLE_ID_SELECTION_INFORMATION_SECTION                       = 0x7F,
};


/* es.c */
typedef enum mpegts_es_type_enum MPEGTSESType;

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

const char* mpegts_es_type_string(MPEGTSESType it);


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

MPEGTSHeader *mpegts_header_new(void);
void          mpegts_header_parse(MPEGTSHeader *it, uint8_t *data);
void          mpegts_header_print_json(MPEGTSHeader *it);
void          mpegts_header_del(MPEGTSHeader* it);


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

	MPEGTSPCR PCR;
};

void mpegts_adaption_parse(MPEGTSAdaption *it, uint8_t *data);
void mpegts_adaption_print_json(MPEGTSAdaption *it);

void mpegts_pcr_parse(MPEGTSPCR *it, uint8_t *data);
void mpegts_pcr_print_json(MPEGTSPCR *it);


/* psi.c */
typedef struct mpegts_PSI_s                      MPEGTSPSI;
typedef struct mpegts_PSI_PAT_s                  MPEGTSPSIPAT;
typedef struct mpegts_PSI_PMT_s                  MPEGTSPSIPMT;
typedef struct mpegts_PSI_PMT_program_element_s  MPEGTSPSIPMTProgramElement;
typedef struct mpegts_PSI_PMT_program_elements_s MPEGTSPSIPMTProgramElements;
typedef struct mpegts_PSI_PMT_ES_info_s          MPEGTSPSIPMTESInfo;
typedef struct mpegts_PSI_PMT_ES_infos_s         MPEGTSPSIPMTESInfos;
typedef struct mpegts_PSI_NIT_s                  MPEGTSPSINIT;
typedef enum   mpegts_PSI_PED_tag_s              MPEGTSPSIPEDTag;

// PD = Program Descriptor
// PED = Program Element Descriptor
enum mpegts_PSI_PED_tag_s {
	MPEGTS_PSI_PED_TAG_RESERVED_00                   = 0x00,
	MPEGTS_PSI_PED_TAG_RESERVED_01                   = 0x01,
	MPEGTS_PSI_PED_TAG_V_H262_13818_11172            = 0x02,
	MPEGTS_PSI_PED_TAG_A_13818_11172                 = 0x03,
	MPEGTS_PSI_PED_TAG_HIERARCHY                     = 0x04,
	MPEGTS_PSI_PED_TAG_REG_PRIVATE                   = 0x05,
	MPEGTS_PSI_PED_TAG_DATA_STREAM_ALIGN             = 0x06,
	MPEGTS_PSI_PED_TAG_GRID                          = 0x07,
	MPEGTS_PSI_PED_TAG_VIDEO_WINDOW                  = 0x08,
	MPEGTS_PSI_PED_TAG_CAS_EMM_ECM_PID               = 0x09,
	MPEGTS_PSI_PED_TAG_ISO_639                       = 0x0A,
	MPEGTS_PSI_PED_TAG_SYSTEM_CLOCK_EXT_REF          = 0x0B,
	MPEGTS_PSI_PED_TAG_MULT_BUF_UTIL_BOUNDS          = 0x0C,
	MPEGTS_PSI_PED_TAG_COPYRIGHT                     = 0x0D,
	MPEGTS_PSI_PED_TAG_MAX_BIT_RATE                  = 0x0E,
	MPEGTS_PSI_PED_TAG_PRIVATE_DATA_INDICATOR        = 0x0F,
	MPEGTS_PSI_PED_TAG_SMOOTHING_BUFFER              = 0x10,
	MPEGTS_PSI_PED_TAG_STD_VIDEO_BUFFER_LEAK_CONTROL = 0x11,
};

#define MPEGTS_PSI_PED_TAG_RESERVED_00_STR "Reserved-00"
#define MPEGTS_PSI_PED_TAG_RESERVED_01_STR "Reserved-01"
#define MPEGTS_PSI_PED_TAG_V_H262_13818_11172_STR "Video stream header parameters for" \
                                                  " ITU-T Rec. H.262," \
                                                  " ISO/IEC 13818-2" \
                                                  " and ISO/IEC 11172-2"
#define MPEGTS_PSI_PED_TAG_A_13818_11172_STR "Audio stream header parameters for" \
                                         " ISO/IEC 13818-3 and ISO/IEC 11172-3"
#define MPEGTS_PSI_PED_TAG_HIERARCHY_STR "Hierarchy for stream selection"
#define MPEGTS_PSI_PED_TAG_REG_PRIVATE_STR "Registration of private formats"
#define MPEGTS_PSI_PED_TAG_DATA_STREAM_ALIGN_STR "Data stream alignment for packetized" \
                                             " video and audio sync point"
#define MPEGTS_PSI_PED_TAG_GRID_STR "Target background grid defines total display area size"
#define MPEGTS_PSI_PED_TAG_VIDEO_WINDOW_STR "Video Window defines position in display area"
#define MPEGTS_PSI_PED_TAG_CAS_EMM_ECM_PID_STR "Conditional access system and EMM/ECM PID"
#define MPEGTS_PSI_PED_TAG_ISO_639_STR "ISO 639 language and audio type"
#define MPEGTS_PSI_PED_TAG_SYSTEM_CLOCK_EXT_REF_STR "System clock external reference"
#define MPEGTS_PSI_PED_TAG_MULT_BUF_UTIL_BOUNDS_STR "Multiplex buffer utilization bounds"
#define MPEGTS_PSI_PED_TAG_COPYRIGHT_STR "Copyright identification system and reference"
#define MPEGTS_PSI_PED_TAG_MAX_BIT_RATE_STR "Maximum bit rate"
#define MPEGTS_PSI_PED_TAG_PRIVATE_DATA_INDICATOR_STR "Private data indicator"
#define MPEGTS_PSI_PED_TAG_SMOOTHING_BUFFER_STR "Smoothing buffer"
#define MPEGTS_PSI_PED_TAG_STD_VIDEO_BUFFER_LEAK_CONTROL_STR "STD video buffer leak control"

// Program Specific Information
struct mpegts_PSI_s {
	// header
	uint8_t
		table_id:8,
		section_syntax_indicator:1,
		private_bit:1,
		reserved_bits:2,
		section_length_unused_bits:2;
	uint16_t section_length:10; // This is a 12-bit field, the first two bits of which shall be "00".
	                            // It specifies the number of bytes of the
	                            // section, starting immediately following
	                            // the section_length field and including the CRC.
	                            // The section_length shall not
	                            // exceed 1 021 so that the entire section has a maximum length
	                            // of 1 024 bytes.
	// table syntax section
	uint16_t transport_stream_id:16;
	uint8_t
		version_number:5,
		curent_next_indicator:1,
		section_number:8,
		last_section_number:8;

	uint32_t CRC32:32;
};

void mpegts_psi_parse(MPEGTSPSI *it, uint8_t *data);
void mpegts_psi_print_json(MPEGTSPSI *it);


/* psi-pat.c */
// PSI -> PAT
// Program association specific data
struct mpegts_PSI_PAT_s {
	MPEGTSPSI psi;

	uint16_t program_number;  // Relates to the Table ID extension in the associated PMT.
	                          // A value of 0 is reserved for a NIT packet identifier.
	uint16_t program_map_PID; // The packet identifier that contains the associated PMT
};

MPEGTSPSIPAT *mpegts_psi_pat_new(void);
void mpegts_psi_pat_parse(MPEGTSPSIPAT *it, uint8_t *data);
void mpegts_psi_pat_print_json(MPEGTSPSIPAT *it);
void mpegts_psi_pat_del(MPEGTSPSIPAT *it);


/* psi-pmt.c */
// PSI -> PMT -> program-element -> ES-info (model)
struct mpegts_PSI_PMT_ES_info_s {
	uint8_t descriptor_tag;
	uint8_t descriptor_length;
	uint8_t descriptor_data[32];
};

// PSI -> PMT -> program-element -> ES-infos (collection)
struct mpegts_PSI_PMT_ES_infos_s {
	uint8_t len;
	uint8_t cap;

	MPEGTSPSIPMTESInfo *c;
};

// PSI -> PMT -> program element (model)
struct mpegts_PSI_PMT_program_element_s {
	uint8_t  stream_type;
	uint16_t elementary_PID;
	uint16_t ES_info_length;

	MPEGTSPSIPMTESInfos es_infos;
};

// PSI -> PMT -> program elements (collection)
struct mpegts_PSI_PMT_program_elements_s {
	uint8_t len;
	uint8_t cap;

	MPEGTSPSIPMTProgramElement *c;
};

// PSI -> PMT
// Program map specific data
struct mpegts_PSI_PMT_s {
	MPEGTSPSI psi;

	uint16_t PCR_PID;
	uint16_t program_info_length;

	MPEGTSPSIPMTProgramElements program_elements;
};

MPEGTSPSIPMT               *mpegts_psi_pmt_new(void);
const char                 *mpegts_psi_ped_tag_string(MPEGTSPSIPEDTag it);
void                        mpegts_psi_pmt_parse(MPEGTSPSIPMT *it, uint8_t *data);
MPEGTSPSIPMTProgramElement *mpegts_psi_pmt_search_by_es_type(MPEGTSPSIPMT *it, MPEGTSESType q);
void                        mpegts_psi_pmt_print_json(MPEGTSPSIPMT *it);
void                        mpegts_psi_pmt_del(MPEGTSPSIPMT *it);


/* psi-cat.c */
// PSI -> CAT
// Conditional access specific data
struct mpegts_CAT_s {
	MPEGTSPSI psi;
};


/* psi-nit.c */
// PSI -> NIT
// network information specific data
struct mpegts_NIT_s {
	MPEGTSPSI psi;
};


/* mpegts.c */
typedef struct mpegts_s MPEGTS;

struct mpegts_s {
	MPEGTSPSIPAT *psi_pat;
	MPEGTSPSIPMT *psi_pmt;
};

MPEGTS *mpegts_new(void);
void    mpegts_del(MPEGTS *it);


#endif // __MPEGTS_MPEGTS__
