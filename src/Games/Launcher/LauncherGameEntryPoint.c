#include "../GameEntryPoint.h"
#include "LauncherCfg.h"

M2Err Launcher_EntryUIButtonHandler(CfgUIButtonType button) {
	if (button == LAUNCHER_CFG_UI_BUTTON_TYPE_ARPG) {
		// TODO
		return M2OK;
	} else if (button == LAUNCHER_CFG_UI_BUTTON_TYPE_RTS) {
		LOG_FATAL("Net yet implemented");
		return M2ERR_QUIT;
	} else {
		return M2ERR_QUIT;
	}
}

M2Err GameEntryPoint_InitLauncher(GameEntryPoint *gep) {
	M2ERR_REFLECT(GameEntryPoint_Init(gep));
	gep->entryUi = &LAUNCHER_CFG_UI_ENTRYPOINT;
	gep->entryUiButtonHandler = Launcher_EntryUIButtonHandler;
	return M2OK;
}
