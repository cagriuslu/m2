#include "../GameEntryPoint.h"
#include "LauncherCfg.h"

M2Err GameEntryPoint_InitLauncher(GameEntryPoint *gep) {
	M2ERR_REFLECT(GameEntryPoint_Init(gep));
	gep->entryUi = &LAUNCHER_CFG_UI_ENTRYPOINT;
	return M2OK;
}
