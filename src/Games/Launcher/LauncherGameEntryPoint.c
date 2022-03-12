#include "../GameProxy.h"
#include "LauncherCfg.h"
#include "../../Game.h"

M2Err Launcher_EntryUIButtonHandler(CfgUIButtonType button) {
	if (button == LAUNCHER_CFG_UI_BUTTONTYP_ARPG) {
		// Terminate yourself
		GameProxy_Term(&GAME->proxy);
		// Load ARPG
		M2Err result = GameProxy_InitARPG(&GAME->proxy);
		if (result) {
			LOG_ERROR_M2(result);
			return result;
		}
		// Execute Entry UI
		result = GameProxy_ExecuteEntryUI(&GAME->proxy);
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

M2Err GameProxy_InitLauncher(GameProxy *gp) {
	M2ERR_REFLECT(GameProxy_Init(gp));
	gp->entryUi = &LAUNCHER_CFG_UI_ENTRYPOINT;
	gp->entryUiButtonHandler = Launcher_EntryUIButtonHandler;
	return M2OK;
}














