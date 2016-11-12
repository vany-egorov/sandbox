#include <stdio.h>
#include <errno.h>      // errno
#include <sysexits.h>   // EX_OK, EX_SOFTWARE
#include <inttypes.h>   // PRId64
#include <unistd.h>     // close
#include <fcntl.h>      // open
#include <string.h>     // memcpy, memset, size_t
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>

#include "io.h"
#include "url.h"
#include "udp.h"
#include "file.h"
#include "fifo.h"
#include "color.h"
#include "config.h"
#include "./mpegts/mpegts.h"
#include "./h264/h264.h"


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

			const char *nal_type_name = h264_nal_type_string(nal_type);

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

typedef struct parse_worker_s ParseWorker;
struct parse_worker_s {
	FIFO    *fifo;

	MPEGTS   mpegts;
	uint8_t  probe;
	uint64_t offset; // global offset
	uint16_t video_PID_H264;
};

void on_msg(ParseWorker *it, uint8_t *msg) {
	int i = 0;
	MPEGTS *mpegts = &it->mpegts;
	MPEGTSPSIPMTProgramElement *mpegts_psi_pmt_program_element = NULL;
	MPEGTSHeader mpegts_header = { 0 };
	MPEGTSAdaption mpegts_adaption = { 0 };
	MPEGTSPSIPMT mpegts_pmt = {0};
	MPEGTSPSI mpegts_psi = { 0 };

	if (msg[0] != MPEGTS_SYNC_BYTE) return;

	mpegts_header_parse(&mpegts_header, &msg[i+1]);

	if (mpegts_header.adaption_field_control) {
		mpegts_adaption_parse(&mpegts_adaption, &msg[i+4]);

		if (mpegts_adaption.PCR_flag)
			mpegts_pcr_print_json(&mpegts_adaption.PCR);
	}

	// PSI-PAT
	if ((!mpegts->psi_pat) &&
	    (mpegts_header.PID == MPEGTS_PID_PAT)) {
		mpegts_psi_pat_del(mpegts->psi_pat);
		mpegts->psi_pat = mpegts_psi_pat_new();
		mpegts_psi_pat_parse(mpegts->psi_pat, &msg[i+5]);

	// PSI-PMT
	} else if ((!mpegts->psi_pmt) &&
	           (mpegts->psi_pat) &&
	           (mpegts_header.PID == mpegts->psi_pat->program_map_PID)) {
		mpegts_psi_pmt_del(mpegts->psi_pmt);
		mpegts->psi_pmt = mpegts_psi_pmt_new();
		mpegts_psi_pmt_parse(mpegts->psi_pmt, &msg[i+5]);
		mpegts_psi_pmt_program_element = mpegts_psi_pmt_search_by_es_type(mpegts->psi_pmt, MPEGTS_STREAM_TYPE_VIDEO_H264);
		if (mpegts_psi_pmt_program_element != NULL)
			it->video_PID_H264 = mpegts_psi_pmt_program_element->elementary_PID;

	} else if ((!mpegts->psi_sdt) &&
	           (mpegts_header.PID == MPEGTS_PID_SDT)) {
		mpegts_psi_sdt_del(mpegts->psi_sdt);
		mpegts->psi_sdt = mpegts_psi_sdt_new();
		mpegts_psi_sdt_parse(mpegts->psi_sdt, &msg[i+5]);
		// printf("SDT: "); psi_print(&psi);
	}

	if (mpegts_header.contains_payload) {
		if (mpegts_header.payload_unit_start_indicator) {
			if (mpegts_header.PID != it->video_PID_H264)
				goto cleanup;

			int pes_offset = i + 4;
			if (mpegts_header.adaption_field_control) {
				pes_offset = pes_offset + mpegts_adaption.adaptation_field_length + 1;
			}

			int pes_length = 0;
			pes_length = MPEGTS_PACKET_SIZE - pes_offset;

			PES pes = { 0 };
			MPEGTSPES mpegts_pes = { 0 };
			pes_parse(&pes, &msg[pes_offset], it->offset, pes_offset, i);
			if (!mpegts_pes_parse(&mpegts_pes, &msg[pes_offset])) {
				mpegts_pes_print_humanized(&mpegts_pes);
				mpegts_pes_print_json(&mpegts_pes);
			}

			// printf("PES 0x%08llX | PTS: %" PRId64 " DTS: %" PRId64 "\n",
			// 	it->offset + pes_offset, pes.PTS, pes.DTS);
		} else {
			int es_offset = i + 4;
			if (mpegts_header.adaption_field_control) {
				es_offset = es_offset + mpegts_adaption.adaptation_field_length + 1;
			}
			int es_length = MPEGTS_PACKET_SIZE - es_offset;
			es_parse(&msg[es_offset], it->offset, es_offset, es_length);
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
	it->offset += MPEGTS_PACKET_SIZE;
}

typedef struct read_worker_s ReadWorker;
struct read_worker_s {
	IOReader *reader;
	IOWriter *writer;
};

void* read_worker_do(void *args) {
	ReadWorker *it = NULL;
	uint8_t buf[MPEGTS_PACKET_COUNT*MPEGTS_PACKET_SIZE] = { 0 };
	size_t copied = 0;

	it = (ReadWorker*)args;

	for(;;)
		io_copy(it->reader, it->writer, buf, sizeof(buf), &copied);

	return NULL;
}

void* parse_worker_do(void *args) {
	int i = 0;
	size_t readed_len = 0,
	       fifo_length = 0;
	ParseWorker *it = (ParseWorker*)args;

	for(;;) {

		fifo_wait_data(it->fifo);
		fifo_length = fifo_len(it->fifo);

		uint8_t msg[MPEGTS_PACKET_SIZE] = { 0 };

		for (i = 0; i < fifo_length; i += MPEGTS_PACKET_SIZE) {
			fifo_read(it->fifo, msg, MPEGTS_PACKET_SIZE, &readed_len);
			on_msg(it, msg);
		}

		if ((it->probe) &&
			  (it->mpegts.psi_pat) &&
		    (it->mpegts.psi_pmt) &&
		    (it->mpegts.psi_sdt)) {
			mpegts_psi_pat_print_humanized(it->mpegts.psi_pat);
			mpegts_psi_sdt_print_humanized(it->mpegts.psi_sdt);
			mpegts_psi_pmt_print_humanized(it->mpegts.psi_pmt);
			exit(0);
		}

	}
}

int main (int argc, char *argv[]) {
	int ret = EX_OK;
	char ebuf[255];

	Config *config = NULL;

	FIFO *fifo = NULL;
	UDP *udp_i = NULL;
	File *file_ts_1 = NULL;
	File *file_ts_2 = NULL;
	IOMultiWriter *multi = NULL;
	IOReader *reader_udp = NULL;
	IOReader *reader_fifo = NULL;
	IOWriter *writer_file_1 = NULL;
	IOWriter *writer_file_2 = NULL;
	IOWriter *writer_fifo = NULL;
	IOWriter *writer_multi = NULL;

	ReadWorker read_worker = { 0 };
	ParseWorker parse_worker = { 0 };
	pthread_t read_thread = { 0 },
	          parse_thread = { 0 };

	config = config_new();
	if (config == NULL) {
		fprintf(stderr, "failed to initialize config\n");
		ret = EX_SOFTWARE; goto cleanup;
	}

	config_parse(config, argc, argv);
	config_print(config);

	if (config_validate(config)) { ret = EX_CONFIG; goto cleanup; }
	if (config->help) { config_help(); goto cleanup; }

	udp_i = udp_new();                     // i
	file_ts_1 = file_new();                // o
	file_ts_2 = file_new();                // o
	fifo = fifo_new(100*7*188);            // o
	multi = io_multi_writer_new(NULL, 0);  // o
	reader_udp = io_reader_new(udp_i, udp_read);
	reader_fifo = io_reader_new(fifo, fifo_read);
	writer_file_1 = io_writer_new(file_ts_1, file_write);
	writer_file_2 = io_writer_new(file_ts_2, file_write);
	writer_fifo = io_writer_new(fifo, fifo_write);
	writer_multi = io_writer_new(multi, io_multi_writer_write);

	if ((!udp_i) ||
		  (!reader_udp) ||
		  (!fifo) ||
		  (!multi) || (!writer_multi) ||
		  (!writer_file_1) || (!writer_file_2) ||
		  (!writer_fifo) || (!reader_fifo)) {
		fprintf(stderr, "error allocating memory for structure\n");
		ret = EX_SOFTWARE; goto cleanup;
	}

	io_multi_writer_push(multi, writer_fifo);
	io_multi_writer_push(multi, writer_file_1);
	io_multi_writer_push(multi, writer_file_2);

	if (udp_connect_i(udp_i, config->i->host, config->i->port, NULL,
	                  ebuf, sizeof(ebuf))) {
		fprintf(stderr, "[udp-i @ %p] connect error: \"%s\"\n", udp_i, ebuf);
		ret = EX_SOFTWARE; goto cleanup;
	} else
		printf("[udp-i @ %p] OK {"
			"\"sock\": %d"
			", \"udp-multicast-group\": \"%s\""
			", \"port\": %d"
			", \"if\": \"%s\""
		"}\n", udp_i, udp_i->sock, config->i->host, config->i->port, "-");

	if (file_open(file_ts_1, "./tmp/out-1.ts", "wb",
	              ebuf, sizeof(ebuf))) {
		fprintf(stderr, "[file-ts-1 @ %p] open error: \"%s\"\n", file_ts_1, ebuf);
		ret = EX_SOFTWARE; goto cleanup;
	} else
		printf("[file-ts-1 @ %p] OK \n", file_ts_1);

	if (file_open(file_ts_2, "./tmp/out-2.ts", "wb",
	              ebuf, sizeof(ebuf))) {
		fprintf(stderr, "[file-ts-2 @ %p] open error: \"%s\"\n", file_ts_2, ebuf);
		ret = EX_SOFTWARE; goto cleanup;
	} else
		printf("[file-ts-2 @ %p] OK \n", file_ts_2);

	read_worker.reader = reader_udp;
	read_worker.writer = writer_multi;
	if (pthread_create(&read_thread, NULL, read_worker_do, (void*)&read_worker)) {
		fprintf(stderr, "pthread-create error: \"%s\"\n", strerror(errno));
		ret = EX_SOFTWARE; goto cleanup;
	} else
		printf("[read-worker @ %p] OK \n", &read_thread);

	// parse_worker.reader = reader_fifo;
	parse_worker.fifo = fifo;
	parse_worker.probe = config->probe;
	if (pthread_create(&parse_thread, NULL, parse_worker_do, (void*)&parse_worker)) {
		fprintf(stderr, "pthread-create error: \"%s\"\n", strerror(errno));
		ret = EX_SOFTWARE; goto cleanup;
	} else
		printf("[parse-worker @ %p] OK \n", &parse_thread);

	pause();

cleanup:
	return ret;
}
