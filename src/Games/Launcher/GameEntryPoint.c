#include "../GameEntryPoint.h"
#include "Cfg.h"

M2Err GameEntryPoint_InitLauncher(GameEntryPoint *gep) {
	M2ERR_REFLECT(GameEntryPoint_Init(gep));
	gep->entryUi = &CFG_UI_LAUNCHER_ENTRYPOINT;
	return M2OK;
}
