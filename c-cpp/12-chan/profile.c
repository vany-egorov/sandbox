#include "profile.h"


void profile_init(Profile *it) {
	it->luma_sz = (size_t)(it->w * it->h);
	it->chroma_sz = it->luma_sz / 4;
	it->yuv_sz = it->luma_sz + 2*it->luma_sz;
}

inline void profile_snprintf(const Profile *it, char *buf, size_t bufsz) {
	snprintf(buf, bufsz, "%dx%d@%d(%s/%s, %zd/%zd/%zd)",
		it->w, it->h, it->b,
		it->preset, it->tune,
		it->luma_sz, it->chroma_sz, it->yuv_sz
	);
}

void profile_print(const Profile *it) {
	char buf[100];
	profile_snprintf(it, buf, sizeof(buf));
	printf("%s\n", buf);
}

inline int profile_is_empty(const Profile *it) {
	if (!it->w && !it->h && !it->b && !it->preset && !it->tune) return 1;
	return 0;
}
