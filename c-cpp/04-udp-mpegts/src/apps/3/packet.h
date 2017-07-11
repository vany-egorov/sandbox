#ifndef __APPS_3_PACKET__
#define __APPS_3_PACKET__


typedef struct packet_s Packet;

struct packet_s {
	int64_t index;
	int64_t offset;
	int64_t PTS;
	int64_t DTS;

	/* buffer */
	/* Buf buf; */
};


#endif  /* __APPS_3_PACKET__ */
