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

#define MPEGTS_SYNC_BYTE 0x47
#define MPEGTS_PACKET_SIZE 188


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

						header_print(&header);
						adaption_print(&adaption);
						pcr_print(&pcr);
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

					fwrite(payload, payload_length, 1, f_h264);
					fflush(f_h264);
					h264_len += payload_length;
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

		fwrite(msg, msg_len, 1, f_ts);
		fflush(f_ts);

		printf("ts   written %d\n", msg_len);
		printf("h264 written %d\n", h264_len);
	}

	fclose(f_h264);
	fclose(f_ts);
}
