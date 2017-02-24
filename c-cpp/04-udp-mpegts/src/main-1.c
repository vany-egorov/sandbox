#include <stdio.h>
#include <errno.h>      // errno
#include <sysexits.h>   // EX_OK, EX_SOFTWARE
#include <inttypes.h>   // PRId64
#include <unistd.h>     // close, pause
#include <fcntl.h>      // open
#include <string.h>     // memcpy, memset, size_t
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>

#include "./io/io.h"
#include "./url/url.h"
#include "./io/udp.h"
#include "./io/file.h"
#include "./db/db.h"
#include "./collections/fifo.h"
#include "./mpegts/mpegts.h"
#include "./h264/h264.h"

#include "color.h"
#include "config.h"


typedef struct parse_worker_s ParseWorker;
struct parse_worker_s {
	FIFO *fifo;

	DB *db;

	MPEGTS   mpegts;
	H264     h264;

	uint8_t  probe;
	uint64_t offset; // global offset
	uint16_t video_PID_H264;
};

// TODO: offset, i, cursor refactoring;
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
	db_store_mpegts_header(it->db, &mpegts_header, it->offset+1);

	if (mpegts_header.adaption_field_control) {
		mpegts_adaption_parse(&mpegts_adaption, &msg[i+4]);
		db_store_mpegts_adaption(it->db, &mpegts_adaption, it->offset+4);

		if (mpegts_adaption.PCR_flag)
			mpegts_pcr_print_json(&mpegts_adaption.PCR);
	}

	// PSI-PAT
	if ((!mpegts->psi_pat) &&
	    (mpegts_header.PID == MPEGTS_PID_PAT)) {
		mpegts_psi_pat_del(mpegts->psi_pat);
		mpegts->psi_pat = mpegts_psi_pat_new();
		mpegts_psi_pat_parse(mpegts->psi_pat, &msg[i+5]);
		db_store_mpegts_psi_pat(it->db, mpegts->psi_pat, it->offset+5);

	// PSI-PMT
	} else if ((!mpegts->psi_pmt) &&
	           (mpegts->psi_pat) &&
	           (mpegts_header.PID == mpegts->psi_pat->program_map_PID)) {
		mpegts_psi_pmt_del(mpegts->psi_pmt);
		mpegts->psi_pmt = mpegts_psi_pmt_new();
		mpegts_psi_pmt_parse(mpegts->psi_pmt, &msg[i+5]);
		db_store_mpegts_psi_pmt(it->db, mpegts->psi_pmt, it->offset+5);
		mpegts_psi_pmt_program_element = mpegts_psi_pmt_search_by_es_type(mpegts->psi_pmt, MPEGTS_STREAM_TYPE_VIDEO_H264);
		if (mpegts_psi_pmt_program_element != NULL)
			it->video_PID_H264 = mpegts_psi_pmt_program_element->elementary_PID;

	} else if ((!mpegts->psi_sdt) &&
	           (mpegts_header.PID == MPEGTS_PID_SDT)) {
		mpegts_psi_sdt_del(mpegts->psi_sdt);
		mpegts->psi_sdt = mpegts_psi_sdt_new();
		mpegts_psi_sdt_parse(mpegts->psi_sdt, &msg[i+5]);
		db_store_mpegts_psi_sdt(it->db, mpegts->psi_sdt, it->offset+5);
	}

	if (mpegts_header.contains_payload) {
		if (mpegts_header.payload_unit_start_indicator) {
			int pes_offset = i + 4;
			if (mpegts_header.adaption_field_control)
				pes_offset = pes_offset + mpegts_adaption.adaptation_field_length + 1;

			MPEGTSPES mpegts_pes = { 0 };
			if (!mpegts_pes_parse(&mpegts_pes, &msg[pes_offset])) {
				// mpegts_pes_print_humanized(&mpegts_pes);
				// mpegts_pes_print_json(&mpegts_pes);
				db_store_mpegts_pes(it->db, &mpegts_pes, it->offset + pes_offset);

				uint8_t *es_data = &msg[pes_offset + 9 + mpegts_pes.header_length];
				int es_offset = pes_offset + 9 + mpegts_pes.header_length;
				int es_length = MPEGTS_PACKET_SIZE - es_offset;

				if (mpegts_header.PID == it->video_PID_H264) {
					H264AnnexBParseResult h264_parse_result = { 0 };
					h264_annexb_parse(&it->h264, &msg[es_offset], (size_t)es_length, it->offset + (uint64_t)es_offset, &h264_parse_result);
					h264_annexb_parse_result_print_humanized_one_line(&h264_parse_result);
					if (h264_parse_result.len) {
						int nal_i = 0;
						for (nal_i = 0; nal_i < h264_parse_result.len; nal_i++) {
							H264NAL *nal = &h264_parse_result.nals[nal_i];
							uint64_t nal_offset = it->offset + (uint64_t)es_offset + (uint64_t)(h264_parse_result.offsets[nal_i]);

							db_store_h264(it->db, nal, nal_offset);
						}
					}
				}
			}
		} else {
			int es_offset = i + 4;
			if (mpegts_header.adaption_field_control)
				es_offset = es_offset + mpegts_adaption.adaptation_field_length + 1;
			int es_length = MPEGTS_PACKET_SIZE - es_offset;

			if (mpegts_header.PID == it->video_PID_H264) {
				H264AnnexBParseResult h264_parse_result = { 0 };
				h264_annexb_parse(&it->h264, &msg[es_offset], (size_t)es_length, it->offset + (uint64_t)es_offset, &h264_parse_result);
				h264_annexb_parse_result_print_humanized_one_line(&h264_parse_result);
				if (h264_parse_result.len) {
					int nal_i = 0;
					for (nal_i = 0; nal_i < h264_parse_result.len; nal_i++) {
						H264NAL *nal = &h264_parse_result.nals[nal_i];
						uint64_t nal_offset = it->offset + (uint64_t)es_offset + (uint64_t)(h264_parse_result.offsets[nal_i]);

						db_store_h264(it->db, nal, nal_offset);
					}
				}
			}
		}
	}

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
		    (it->mpegts.psi_sdt) &&
		    (it->h264.got_nal_sps) &&
		    (it->h264.got_nal_pps) &&
		    (it->h264.got_nal_aud)) {
			mpegts_psi_pat_print_humanized(it->mpegts.psi_pat);
			mpegts_psi_sdt_print_humanized(it->mpegts.psi_sdt);
			mpegts_psi_pmt_print_humanized(it->mpegts.psi_pmt);
			h264_nal_sps_print_humanized(&it->h264.nal_sps);
			h264_nal_pps_print_humanized(&it->h264.nal_pps);
			h264_nal_aud_print_humanized(&it->h264.nal_aud);

			MPEGTSHeader *mpegts_header = NULL;
			MPEGTSAdaption *mpegts_adaption = NULL;
			MPEGTSPES *mpegts_pes = NULL;

			for (i = 0; i < it->db->atoms->len; i++) {
				DBAtom *atom = slice_get(it->db->atoms, i);
				switch (atom->kind) {
				case DB_MPEGTS_HEADER:
					mpegts_header = (MPEGTSHeader*)atom->data;
					// mpegts_header_print_json(mpegts_header);
					break;
				case DB_MPEGTS_ADAPTION:
					mpegts_adaption = (MPEGTSAdaption*)atom->data;
					mpegts_adaption_print_json(mpegts_adaption);
					if (mpegts_adaption->PCR_flag)
						mpegts_pcr_print_json(&mpegts_adaption->PCR);
					break;
				case DB_MPEGTS_PES:
					mpegts_pes = (MPEGTSPES*)atom->data;
					mpegts_pes_print_humanized(mpegts_pes);
					break;
				}
			}

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

	if (config->help) { config_help(); goto cleanup; }
	if (config_validate(config)) { ret = EX_CONFIG; goto cleanup; }

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

	if (udp_connect_i(udp_i, url_host(config->i), config->i->port, NULL,
	                  ebuf, sizeof(ebuf))) {
		fprintf(stderr, "[udp-i @ %p] connect error: \"%s\"\n", udp_i, ebuf);
		ret = EX_SOFTWARE; goto cleanup;
	} else
		printf("[udp-i @ %p] OK {"
			"\"sock\": %d"
			", \"udp-multicast-group\": \"%s\""
			", \"port\": %d"
			", \"if\": \"%s\""
		"}\n", udp_i, udp_i->sock, url_host(config->i), config->i->port, "-");

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
	db_new(&parse_worker.db);
	if (pthread_create(&parse_thread, NULL, parse_worker_do, (void*)&parse_worker)) {
		fprintf(stderr, "pthread-create error: \"%s\"\n", strerror(errno));
		ret = EX_SOFTWARE; goto cleanup;
	} else
		printf("[parse-worker @ %p] OK \n", &parse_thread);

	pause();

cleanup:
	return ret;
}
