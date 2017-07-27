#ifndef __VA_COMMON_FN__
#define __VA_COMMON_FN__


#include <mpegts/mpegts.h>  /* MPEGTSPSIPMTProgramElement */

#include "codec-kind.h"  /* CodecKind */


CodecKind fn_codec_kind_from_mpegts_pmt_pe(MPEGTSPSIPMTProgramElement *pe);


#endif /* __VA_COMMON_FN__ */
