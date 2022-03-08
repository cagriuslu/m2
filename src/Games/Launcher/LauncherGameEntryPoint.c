#include "../GameEntryPoint.h"
#include "LauncherCfg.h"
#include "../../Game.h"

M2Err Launcher_EntryUIButtonHandler(CfgUIButtonType button) {
	if (button == LAUNCHER_CFG_UI_BUTTONTYP_ARPG) {
		// Terminate yourself
		GameEntryPoint_Term(&GAME->gameEntryPoint);
		// Load ARPG
		M2Err result = GameEntryPoint_InitARPG(&GAME->gameEntryPoint);
		if (result) {
			LOG_ERROR_M2(result);
			return result;
		}
		// Execute Entry UI
		result = GameEntryPoint_ExecuteEntryUI(&GAME->gameEntryPoint);
		if (result) {
			LOG_ERROR_M2(result);
			return result;
		}
		return M2OK;
	} else if (button == LAUNCHER_CFG_UI_BUTTONTYP_RTS) {
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














