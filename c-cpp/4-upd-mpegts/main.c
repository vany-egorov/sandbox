#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>     // close
#include <fcntl.h>      // open
#include <string.h>     // memset
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>


#define EXAMPLE_PORT 5500
#define EXAMPLE_GROUP "239.1.1.1"
#define MSG_SIZE 1504

#define MPEGTS_SYNC_BYTE    0x47
#define MPEGTS_PACKET_SIZE  188

#define MPEGTS_PID_PAT      0x0000
#define MPEGTS_PID_CAT      0x0001
#define MPEGTS_PID_TSDT     0x0002
#define MPEGTS_PID_CIT      0x0003
#define MPEGTS_PID_NULL     0x1FFF

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
#define MPEGTS_STREAM_TYPE_VIDEO_H265          0x42


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

int main (int argc, char *argv[]) {
	struct sockaddr_in addr;
	int addrlen, sock, msg_len, i, j, h264_len;
	struct ip_mreq mreq;
	uint8_t msg[MSG_SIZE];
	FILE *f_h264 = fopen("./tmp/out.264", "ab");
	FILE *f_ts = fopen("./tmp/out.ts", "ab");

	/* set up socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
	 perror("socket");
	 exit(1);
	}
	bzero((char *)&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(EXAMPLE_PORT);
	addrlen = sizeof(addr);

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		exit(1);
	}
	mreq.imr_multiaddr.s_addr = inet_addr(EXAMPLE_GROUP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
		perror("setsockopt mreq");
		exit(1);
	}

	uint16_t program_map_PID = 0;

	while (1) {
		memset(msg, 0, sizeof msg);
		msg_len = 0;
		h264_len = 0;
		msg_len = recvfrom(sock, msg, sizeof(msg), 0, (struct sockaddr *) &addr, &addrlen);
		if (msg_len < 0) {
			perror("recvfrom");
			exit(1);
		} else if (msg_len == 0) {
			break;
		}

		for (i = 0; i < msg_len; i++) {
			if ((msg[i] == MPEGTS_SYNC_BYTE) && !(i % MPEGTS_PACKET_SIZE)) {
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
					// PSI - hader
					uint8_t table_id = (uint8_t)msg[i+5];
					uint8_t section_syntax_indicator = !!( (uint8_t)msg[i+6] & 0x80 );
					uint8_t private_bit = !!( (uint8_t)msg[i+6] & 0x40 );
					uint8_t reserved_bits = (uint8_t)msg[i+6] & 0x30;
					uint8_t section_length_unused_bits = (uint8_t)msg[i+6] & 0x0C;
					uint16_t section_length = (((uint16_t)msg[i+6] & 0x03 ) << 8) | ((uint16_t)msg[i+7] & 0xFF);

					// PSI - table syntax section
					uint16_t transport_stream_id = (
						(((uint16_t)msg[i+8] & 0xFF) << 8) |
						((uint16_t)msg[i+9] & 0xFF)
					);
					uint8_t version_number = (uint8_t)msg[i+10] & 0x3E;
					uint8_t curent_next_indicator = (uint8_t)msg[i+10] & 0x01;
					uint8_t section_number = (uint8_t)msg[i+11];
					uint8_t last_section_number = (uint8_t)msg[i+12];
					uint16_t CRC32_i = (uint16_t)i + 7 + section_length;
					uint32_t CRC32 = (
						(((uint32_t)msg[CRC32_i-3] & 0xFF) << 24) |
						(((uint32_t)msg[CRC32_i-2] & 0xFF) << 16) |
						(((uint32_t)msg[CRC32_i-1] & 0xFF) << 8) |
						((uint32_t)msg[CRC32_i] & 0xFF)
					);
					// printf("0x%02X | %d | %d | %d | %d | %d | %d | %d | %d | 0x%04x\n", table_id, section_syntax_indicator, private_bit, section_length, transport_stream_id, version_number, curent_next_indicator, section_number, last_section_number, CRC32);

					if (table_id == MPEGTS_TABLE_ID_PROGRAM_ASSOCIATION_SECTION) {
						// PAT (Program association specific data)
						uint16_t program_number = (((uint16_t)msg[i+13] & 0xFF) << 8) | ((uint16_t)msg[i+14] & 0xFF);
						program_map_PID = (((uint16_t)msg[i+15] & 0x1F) << 8) | ((uint16_t)msg[i+16] & 0xFF);
						// printf("%d 0x%04x %d\n", program_number, program_map_PID, program_map_PID);
					}
				}
				if ((program_map_PID) && (header.PID == program_map_PID)) {
					// PSI - hader
					uint8_t table_id = (uint8_t)msg[i+5];
					uint8_t section_syntax_indicator = !!( (uint8_t)msg[i+6] & 0x80 );
					uint8_t private_bit = !!( (uint8_t)msg[i+6] & 0x40 );
					uint8_t reserved_bits = (uint8_t)msg[i+6] & 0x30;
					uint8_t section_length_unused_bits = (uint8_t)msg[i+6] & 0x0C;
					uint16_t section_length = (((uint16_t)msg[i+6] & 0x03 ) << 8) | ((uint16_t)msg[i+7] & 0xFF);

					// PSI - table syntax section
					uint16_t transport_stream_id = (
						(((uint16_t)msg[i+8] & 0xFF) << 8) |
						((uint16_t)msg[i+9] & 0xFF)
					);
					uint8_t version_number = (uint8_t)msg[i+10] & 0x3E;
					uint8_t curent_next_indicator = (uint8_t)msg[i+10] & 0x01;
					uint8_t section_number = (uint8_t)msg[i+11];
					uint8_t last_section_number = (uint8_t)msg[i+12];
					uint16_t CRC32_i = (uint16_t)i + 7 + section_length;
					uint32_t CRC32 = (
						(((uint32_t)msg[CRC32_i-3] & 0xFF) << 24) |
						(((uint32_t)msg[CRC32_i-2] & 0xFF) << 16) |
						(((uint32_t)msg[CRC32_i-1] & 0xFF) << 8) |
						((uint32_t)msg[CRC32_i] & 0xFF)
					);
					// printf("0x%02X | %d | %d | %d | %d | %d | %d | %d | %d | 0x%04x\n", table_id, section_syntax_indicator, private_bit, section_length, transport_stream_id, version_number, curent_next_indicator, section_number, last_section_number, CRC32);

					if (table_id == MPEGTS_TABLE_ID_PROGRAM_MAP_SECTION) {
						int16_t section_length_unreaded = (int16_t)section_length;
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
						printf("%d | %d | %d\n", PCR_PID, program_info_length, section_length_unreaded);

						int pmt_start = i + 17;
						int pmt_offset = 0;
						while (section_length_unreaded > 0) {
							uint8_t stream_type = (uint8_t)msg[pmt_start+pmt_offset];
							uint16_t elementary_PID = (
								(((uint16_t)msg[pmt_start+1+pmt_offset] & 0x1F) << 8) |
								((uint16_t)msg[pmt_start+2+pmt_offset] & 0xFF)
							);
							uint16_t ES_info_length = (
								(((uint16_t)msg[pmt_start+3+pmt_offset] & 0x03) << 8) |
								((uint16_t)msg[pmt_start+4+pmt_offset] & 0xFF)
							);
							section_length_unreaded -= (5 + ES_info_length);
							pmt_offset += (5 + ES_info_length);
							printf("- 0x%02x | 0x%02x | %d | %d\n", stream_type, elementary_PID, ES_info_length, section_length);
						}
					}
				}

				if (header.contains_payload) {
					if (header.PID !=0x100)
						continue;

					uint8_t payload_offset = i+4;
					if (header.adaption_field_control) {
						payload_offset = payload_offset + 1 + adaption.adaptation_field_length;
					}

					uint8_t *payload = &msg[payload_offset];
					uint8_t payload_length = 0;
					payload_length = MPEGTS_PACKET_SIZE - payload_offset;

					// printf("payload_offset - %d\n", payload_offset);
					// printf("payload_length - %d\n", payload_length);

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
		}

		// fwrite(msg, msg_len, 1, f_ts);
		// fflush(f_ts);

		// printf("ts   written %d\n", msg_len);
		// printf("h264 written %d\n", h264_len);
	}

	fclose(f_h264);
	fclose(f_ts);
}
