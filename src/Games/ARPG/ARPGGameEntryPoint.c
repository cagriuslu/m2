#include "../GameEntryPoint.h"
#include "ARPGCfg.h"
#include "../../Game.h"

static M2Err ARPG_EntryUIButtonHandler(CfgUIButtonType button) {
	if (button == ARPG_CFG_UI_BUTTONTYPE_NEWGAME) {
		M2ERR_REFLECT(Game_Level_Init());
		M2ERR_REFLECT(Game_Level_Load(&CFG_LVL_SP000));
		M2ERR_REFLECT(PathfinderMap_Init(&GAME->pathfinderMap));
		return M2OK;
	} else {
		return M2ERR_QUIT;
	}
}

static M2Err ARPG_PauseUIButtonHandler(CfgUIButtonType button) {
	if (button == ARPG_CFG_UI_BUTTONTYPE_RESUME) {
		return M2OK;
	} else {
		return M2ERR_QUIT;
	}
}

M2Err GameEntryPoint_InitARPG(GameEntryPoint *gep) {
	M2ERR_REFLECT(GameEntryPoint_Init(gep));
	gep->entryUi = &ARPG_CFG_UI_ENTRYUI;
	gep->entryUiButtonHandler = ARPG_EntryUIButtonHandler;
	gep->pauseUi = &ARPG_CFG_UI_PAUSEUI;
	gep->pauseUiButtonHandler = ARPG_PauseUIButtonHandler;
	return M2OK;
}
