#ifndef __APPS_3_WRKR_CFG__
#define __APPS_3_WRKR_CFG__


#include <collections/slice.h>  /* InputCfg */

#include "map.h"    /* Map */
#include "input.h"  /* InputCfg */


typedef struct wrkr_cfg_s        WrkrCfg;
typedef struct wrkr_map_cfg_s    WrkrMapCfg;
typedef struct wrkr_output_cfg_s WrkrOutputCfg;


struct wrkr_cfg_s {
	URL url;

	InputCfg i;  /* input */
	Slice    m;  /* []MapCfg */
};

struct wrkr_map_cfg_s {
	Map m;
	Slice o;  /* []Output */
};

struct wrkr_output_cfg_s {
	URL u;
};


int wrkr_cfg_init(WrkrCfg *it);
int wrkr_cfg_fin(WrkrCfg *it);

int wrkr_map_cfg_init(WrkrMapCfg *it);
int wrkr_map_cfg_fin(WrkrMapCfg *it);


#endif /* __APPS_3_WRKR_CFG__ */
