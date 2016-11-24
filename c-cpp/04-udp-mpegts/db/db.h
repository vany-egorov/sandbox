#ifndef __DB_DB__
#define __DB_DB__


#include "../collections/slice.h"


typedef struct db_s              DB;
typedef struct db_atom_s         DBAtom;
typedef enum   db_atom_kind_enum DBAtomKind;

struct db_s {
	struct timespec start_at;

	Slice *atoms;

	Slice *mpegts_tss;
	Slice *mpegts_pess;
	Slice *mpegts_psi_pats;
	Slice *mpegts_psi_pmts;
	Slice *mpegts_psi_sdts;

	Slice *h264_auds;
	Slice *h264_seis;
	Slice *h264_spss;
	Slice *h264_ppss;
	Slice *h264_slice_idrs;
};

enum db_atom_kind_enum {
	MPEGTS_TS,
	MPEGTS_PES,
	MPEGTS_PSI_PAT,
	MPEGTS_PSI_PMT,
	MPEGTS_PSI_SDT,

	H264_AUD,
	H264_SEI,
	H264_SPS,
	H264_PPS,
	H264_SLICE_IDR,
};

struct db_atom_s {
	DBAtomKind kind;

	void *data;
};


#endif // __DB_DB__
