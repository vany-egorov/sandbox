#ifndef __MPEGTS_MPEGTS__
#define __MPEGTS_MPEGTS__


typedef struct mpegts_adaption_s MPEGTSAdaption;

void mpegts_adaption_parse(MPEGTSAdaption *it, uint8_t *data);


#endif // __MPEGTS_MPEGTS__
