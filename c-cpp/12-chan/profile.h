#ifndef __PROFILE_H__
#define __PROFILE_H__


#include <stdio.h>   /* printf, snprintf */
#include <stdint.h>  /* uint16_t */
#include <stddef.h>  /* size_t */


typedef struct profile_s Profile;


struct profile_s{
	uint16_t w;        /* width */
	uint16_t h;        /* height */
	uint16_t b;        /* bitrate */
	uint8_t  bframes;  /* */

	char *preset;
	char *tune;

	size_t luma_sz;
	size_t chroma_sz;
	size_t yuv_sz;
};


void profile_init(Profile *it);
inline void profile_snprintf(const Profile *it, char *buf, size_t bufsz);
void profile_print(const Profile *it);
inline int profile_is_empty(const Profile *it);


#endif /* __PROFILE_H__ */
