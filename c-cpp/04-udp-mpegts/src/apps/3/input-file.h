#ifndef __APPS_3_INPUT_FILE__
#define __APPS_3_INPUT_FILE__


#include <io/file.h>        /* File */
#include <url/url.h>        /* URL */
#include <mpegts/mpegts.h>  /* MPEGTS_PACKET_SIZE */

#include "input-logger.h"  /* input_logger */
#include "input-vt.h"      /* InputVT, input_read_cb_fn */


typedef struct input_file_s InputFile;
typedef struct input_file_cfg_s InputFileCfg;


extern InputVT input_file_vt;   /* UDP virtual table */


struct input_file_cfg_s { };

struct input_file_s {
	/* TODO: inherit from intput? */
	File i;
	InputFileCfg *c;
};

int input_file_new(InputFile **out);


#endif /* __APPS_3_INPUT_FILE__ */
