#ifndef __APPS_3_FILTER__
#define __APPS_3_FILTER__


#include <stdlib.h>  /* calloc */

#include <collections/slice.h>  /* Slice */

#include "stream.h"  /* Stream */
#include "track.h"   /* Track */
#include "packet.h"  /* Packet */
#include "frame.h"   /* Frame */


typedef struct filter_s Filter;
typedef struct filter_vt_s FilterVT;


struct filter_s {
	Slice consumers;

	char *name;

	void *w; /* wrapped, child, opaque */

	FilterVT *vt; /* virtual table */
};

struct filter_vt_s {
	int (*consume_strm) (void *ctx, Stream *strm);
	int (*consume_trk) (void *ctx, Track *trk);
	int (*consume_pkt) (void *ctx, Packet *pkt);
	int (*consume_pkt_raw) (void *ctx, uint8_t *buf, size_t bufsz);
	int (*consume_frm) (void *ctx , Frame *frm);
};


int filter_new(Filter **out);
int filter_init(Filter *it);
int filter_append_consumer(Filter *it, Filter *cnsmr);
int filter_fin(Filter *it);
int filter_del(Filter **out);

int filter_consume_strm(Filter *it, Stream *strm);
int filter_consume_trk(Filter *it, Track *trk);
int filter_consume_pkt(Filter *it, Packet *pkt);
int filter_consume_pkt_raw(Filter *it, uint8_t *buf, size_t bufsz);
int filter_consume_frm(Filter *it, Frame *frm);

int filter_produce_strm(Filter *it, Stream *strm);
int filter_produce_trk(Filter *it, Track *trk);
int filter_produce_pkt(Filter *it, Packet *pkt);
int filter_produce_pkt_raw(Filter *it, uint8_t *buf, size_t bufsz);
int filter_produce_frm(Filter *it, Frame *frm);


#endif /* __APPS_3_FILTER__ */
