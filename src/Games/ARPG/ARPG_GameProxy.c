#include <m2/GameProxy.h>
#include <m2/Game.h>
#include "ARPG_Cfg.h"
#include "ARPG_Object.h"
#include "ARPG_Component.h"
#include "ARPG_UIElement.h"

static M2Err ARPG_EntryUIButtonHandler(CfgUIButtonType button) {
	if (button == ARPG_CFG_UI_BUTTONTYPE_NEWGAME) {
		return Game_Level_Load(&CFG_LVL_SP000);
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

static M2Err ARPG_ForegroundSpriteLoader(Object *obj, CfgSpriteIndex idx, Vec2F position) {
	switch (idx) {
		case ARPG_CFGSPRITE_PLAYER_LOOKDOWN_00:
			return ObjectPlayer_InitFromCfg(obj, &CFG_CHARACTER_PLAYER, position);
		case ARPG_CFGSPRITE_ENEMY_LOOKDOWN_00:
			return ObjectEnemy_InitFromCfg(obj, &CFG_CHARACTER_SKELETON_000_CHASE, position);
		default:
			return LOG_ERROR_M2V(M2ERR_INVALID_CFG_OBJTYP, Int32, idx);
	}
}

M2Err GameProxy_InitARPG(GameProxy *gp) {
	M2ERR_REFLECT(GameProxy_Init(gp));
	gp->entryUi = &ARPG_CFG_UI_ENTRYUI;
	gp->entryUiButtonHandler = ARPG_EntryUIButtonHandler;
	gp->pauseUi = &ARPG_CFG_UI_PAUSEUI;
	gp->pauseUiButtonHandler = ARPG_PauseUIButtonHandler;
	gp->tileSize = ARPG_CFG_TILE_SIZE;
	gp->textureMapFile = "resources/48.png";
	gp->textureMaskFile = "resources/48-Mask.png";
	gp->cfgSprites = ARPG_CFG_SPRITES;
	gp->cfgSpriteCount = ARPG_CFGSPRITE_N;
	gp->cfgHUDLeft = &CFG_UI_HUDLEFT;
	gp->cfgHUDRight = &CFG_UI_HUDRIGHT;
	gp->uiElementUpdateDynamic = UIElement_UpdateDynamic;
	gp->componentDefenseDataSize = sizeof(ARPG_ComponentDefense);
	gp->componentOffenseDataSize = sizeof(ARPG_ComponentOffense);
	gp->foregroundSpriteLoader = ARPG_ForegroundSpriteLoader;
	return M2OK;
}
