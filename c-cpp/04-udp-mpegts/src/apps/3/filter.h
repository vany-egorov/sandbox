#ifndef __APPS_3_FILTER__
#define __APPS_3_FILTER__


typedef struct filter_s Filter;


struct filter_s {
	// Slice *consumers;
};


void filter_add_consumer(void);

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
