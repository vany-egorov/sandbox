#include "wrkr-cfg.h"


int wrkr_cfg_init(WrkrCfg *it) {
	slice_init(&it->m, sizeof(PipelineMapCfg));
	return 0;
}

int wrkr_cfg_fin(WrkrCfg *it) {
	slice_fin(&it->m);
	return 0;
}

int pipeline_map_cfg_init(PipelineMapCfg *it) {
	slice_init(&it->o, sizeof(PipelineOutputCfg));
	return 0;
}

int pipeline_map_cfg_fin(PipelineMapCfg *it) {
	slice_fin(&it->o);
	return 0;
}
