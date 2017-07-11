#ifndef __APPS_3_PACKET__
#define __APPS_3_PACKET__


typedef struct packet_s Packet;

struct packet_s {
	/* offset inside stream */
	int64_t offset;

	/* presentation time stamp */
	int64_t PTS;

	/* decode time stamp */
	int64_t DTS;

	/* buffer */
	/* Buf buf; */
};


#endif  /* __APPS_3_PACKET__ */
