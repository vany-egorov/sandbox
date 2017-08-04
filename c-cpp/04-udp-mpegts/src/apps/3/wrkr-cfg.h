#ifndef __APPS_3_WRKR_CFG__
#define __APPS_3_WRKR_CFG__


#include <collections/slice.h>  /* InputCfg */

#include "map.h"    /* Map */
#include "input.h"  /* InputCfg */


typedef struct wrkr_cfg_s            WrkrCfg;
typedef struct pipeline_map_cfg_s    PipelineMapCfg;
typedef struct pipeline_output_cfg_s PipelineOutputCfg;


struct wrkr_cfg_s {
	URL url;

	InputCfg i;  /* input */
	Slice    m;  /* []PipelineMapCfg */
};

struct pipeline_map_cfg_s {
	Map m;
	Slice o;  /* []PipelineOutputCfg */
};

struct pipeline_output_cfg_s {
	URL u;
};


int wrkr_cfg_init(WrkrCfg *it);
int wrkr_cfg_fin(WrkrCfg *it);

int pipeline_map_cfg_init(PipelineMapCfg *it);
int pipeline_map_cfg_fin(PipelineMapCfg *it);


#endif /* __APPS_3_WRKR_CFG__ */
