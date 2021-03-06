#ifndef __APPS_3_INPUT_UDP__
#define __APPS_3_INPUT_UDP__


#include <io/io.h>             /* IOReader, IOWriter */
#include <io/udp.h>            /* UDP */
#include <url/url.h>           /* URL */
#include <mpegts/mpegts.h>     /* MPEGTS_PACKET_COUNT, MPEGTS_PACKET_SIZE */
#include <collections/fifo.h>  /* FIFO */

#include "input-logger.h"  /* input_logger */
#include "input-vt.h"      /* InputVT, input_read_cb_fn */


typedef struct input_udp_s InputUDP;
typedef struct input_udp_cfg_s InputUDPCfg;


extern InputVT input_udp_vt;  /* file virtual table */


struct input_udp_cfg_s {
	/* fifo capacity */
	size_t fifo_cap;

	/* buffer size while reading data from
	 * udp source
	 */
	size_t fifo_read_buf_sz;
};

struct input_udp_s {
	/* TODO: inherit from intput? */

	/* public */
	UDP i;
	InputUDPCfg *c;

	FIFO fifo;

	/* private */
	pthread_t _thrd;
};

int input_udp_new(InputUDP **out);
int input_udp_init(InputUDP *it, InputUDPCfg *c);


#endif /* __APPS_3_INPUT_UDP__ */
