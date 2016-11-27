#ifndef __DB_DB__
#define __DB_DB__


#include "../mpegts/mpegts.h"
#include "../collections/slice.h"


typedef struct db_s              DB;
typedef struct db_timestamp_s    DBTimestamp;
typedef struct db_atom_s         DBAtom;
typedef enum   db_atom_kind_enum DBAtomKind;

struct db_s {
	time_t start_at;

	Slice *timestamps;

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

struct db_timestamp_s {
	void *l; /* left */
	void *r; /* right */
};

enum db_atom_kind_enum {
	DB_MPEGTS_TS,
	DB_MPEGTS_PES,
	DB_MPEGTS_PSI_PAT,
	DB_MPEGTS_PSI_PMT,
	DB_MPEGTS_PSI_SDT,

	DB_H264_AUD,
	DB_H264_SEI,
	DB_H264_SPS,
	DB_H264_PPS,
	DB_H264_SLICE_IDR,
};

struct db_atom_s {
	DBAtomKind kind;

	// global offset inside stream
	uint64_t offset;

	void *data;
};

int db_new(DB **out);
int db_store_mpegts_pes(DB *it, MPEGTSPES *item);
int db_store_h264_sps(DB *it);
void db_del(DB **it);


#endif // __DB_DB__
