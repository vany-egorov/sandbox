#ifndef __DB_DB__
#define __DB_DB__


#include "../mpegts/mpegts.h"
#include "../h264/h264.h"
#include "../collections/slice.h"


typedef struct db_s              DB;
typedef struct db_timestamp_s    DBTimestamp;
typedef struct db_atom_s         DBAtom;
typedef enum   db_atom_kind_enum DBAtomKind;

struct db_s {
	time_t start_at;

	Slice *timestamps;

	Slice *atoms;

	Slice *mpegts_headers;
	Slice *mpegts_adaptions;
	Slice *mpegts_pess;
	Slice *mpegts_psi_pats;
	Slice *mpegts_psi_pmts;
	Slice *mpegts_psi_sdts;

	Slice *h264_spss;
	Slice *h264_ppss;
	Slice *h264_auds;
	Slice *h264_seis;
	Slice *h264_slice_idrs;
};

struct db_timestamp_s {
	void *l; /* left */
	void *r; /* right */
};

enum db_atom_kind_enum {
	DB_MPEGTS_HEADER,
	DB_MPEGTS_ADAPTION,
	DB_MPEGTS_PES,
	DB_MPEGTS_PSI_PAT,
	DB_MPEGTS_PSI_PMT,
	DB_MPEGTS_PSI_SDT,

	DB_H264_SPS,
	DB_H264_PPS,
	DB_H264_AUD,
	DB_H264_SEI,
	DB_H264_SLICE_IDR,
};

struct db_atom_s {
	DBAtomKind kind;

	/* global offset inside stream */
	uint64_t offset;

	void *data;
};


int db_new(DB **out);

int db_store_mpegts_header  (DB *it, MPEGTSHeader   *item, uint64_t offset);  /* TS transport packet */
int db_store_mpegts_adaption(DB *it, MPEGTSAdaption *item, uint64_t offset);  /* TS Adaption */
int db_store_mpegts_pes     (DB *it, MPEGTSPES      *item, uint64_t offset);  /* PES packet */
int db_store_mpegts_psi_pat (DB *it, MPEGTSPSIPAT   *item, uint64_t offset);  /* Program Association Table */
int db_store_mpegts_psi_pmt (DB *it, MPEGTSPSIPMT   *item, uint64_t offset);  /* Program Map Table */
int db_store_mpegts_psi_sdt (DB *it, MPEGTSPSISDT   *item, uint64_t offset);  /* Service Description Table */

int db_store_h264          (DB *it, H264NAL         *nal,  uint64_t offset);
int db_store_h264_sps      (DB *it, H264NALSPS      *item, uint64_t offset); /* H264 Sequence Parameter Set */
int db_store_h264_pps      (DB *it, H264NALPPS      *item, uint64_t offset); /* H264 Picture Parameter Set */
int db_store_h264_aud      (DB *it, H264NALAUD      *item, uint64_t offset); /* H264 AUD */
int db_store_h264_sei      (DB *it, H264NALSEI      *item, uint64_t offset); /* H264 SEI */
int db_store_h264_slice_idr(DB *it, H264NALSliceIDR *item, uint64_t offset); /* H264 I/P/B Slice */

void db_del(DB **it);


#endif /* __DB_DB__ */
