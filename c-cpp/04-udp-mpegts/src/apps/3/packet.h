#ifndef __APPS_3_PACKET__
#define __APPS_3_PACKET__


#include <io/buf.h>  /* buf */


typedef struct packet_s Packet;
typedef struct track_s  Track;

struct packet_s {
	Track *trk;

	/* offset inside stream */
	int64_t offset;

	/* presentation time stamp */
	int64_t PTS;

	/* decode time stamp */
	int64_t DTS;

	/* buffer */
	Buf buf;
};


int packet_init(Packet *it);
int packet_fin(Packet *it);


#endif  /* __APPS_3_PACKET__ */
