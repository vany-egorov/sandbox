#include "mpegts.h"


MPEGTS *mpegts_new(void) {
	MPEGTS *it = calloc(1, sizeof(MPEGTS));

	it->psi_pat = NULL;
	it->psi_pmt = NULL;

	return it;
}

void mpegts_del(MPEGTS *it) {
	if (!it) return;

	if (it->psi_pat) {
		mpegts_psi_pat_del(it->psi_pat);
		it->psi_pat = NULL;
	}

	if (it->psi_pmt) {
		mpegts_psi_pmt_del(it->psi_pmt);
		it->psi_pmt = NULL;
	}

	free(it);
}
