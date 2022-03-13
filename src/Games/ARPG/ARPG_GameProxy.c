#include "../GameProxy.h"
#include "ARPG_Cfg.h"
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

M2Err GameProxy_InitARPG(GameProxy *gp) {
	M2ERR_REFLECT(GameProxy_Init(gp));
	gp->entryUi = &ARPG_CFG_UI_ENTRYUI;
	gp->entryUiButtonHandler = ARPG_EntryUIButtonHandler;
	gp->tileSize = ARPG_CFG_TILE_SIZE;
	gp->pauseUi = &ARPG_CFG_UI_PAUSEUI;
	gp->pauseUiButtonHandler = ARPG_PauseUIButtonHandler;
	return M2OK;
}
