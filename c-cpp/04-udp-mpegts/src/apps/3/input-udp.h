#ifndef __APPS_3_INPUT_UDP__
#define __APPS_3_INPUT_UDP__


#include <io/io.h>             /* IOReader, IOWriter */
#include <io/udp.h>            /* UDP */
#include <url/url.h>           /* URL */
#include <mpegts/mpegts.h>     /* MPEGTS_PACKET_COUNT, MPEGTS_PACKET_SIZE */
#include <collections/fifo.h>  /* FIFO */


typedef struct input_udp_s InputUDP;

struct input_udp_s {
	UDP i;
	FIFO fifo;

	pthread_t _thrd;
};

int input_udp_new(InputUDP **out);


#endif /* __APPS_3_INPUT_UDP__ */
