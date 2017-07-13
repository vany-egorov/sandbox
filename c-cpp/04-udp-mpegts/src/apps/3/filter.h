#ifndef __APPS_3_FILTER__
#define __APPS_3_FILTER__


typedef struct filter_s Filter;
typedef struct filter_vt_s FilterVT;


struct filter_s {
	Slice *consumers;

	void *w; /* wrapped, child, opaque */

	FilterVT *vt; /* virtual table */
};

struct filter_vt_s {
	int (*filter_consume_stream) (void *ctx, Stream *strm);
	int (*filter_consume_track) (void *ctx, Stream *trk);
	int (*filter_consume_packet) (void *ctx, Packet *pkt);
	int (*filter_consume_packet_raw) (void *ctx, uint8_t *buf, size_t bufsz);
};


void filter_add_consumer(Filter *it);

void filter_produce_stream(void);
void filter_produce_track(void);
void filter_produce_packet(void);
void filter_produce_packet_raw(void);
void filter_produce_frame(void);

void filter_consume_stream(void);
void filter_consume_track(void);
void filter_consume_packet(void);
void filter_consume_packet_raw(void);
void filter_consume_frame(void);


#endif /* __APPS_3_FILTER__ */
