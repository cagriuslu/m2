#include <impl/game_proxy.hh>
#include <impl/ARPG_Cfg.hh>
#include "ARPG_Object.hh"
#include "ARPG_UIElement.hh"
#include <impl/object/Enemy.h>
#include "m2/Game.hh"

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

static M2Err ARPG_ForegroundSpriteLoader(m2::Object *obj, CfgSpriteIndex idx, m2::Vec2f position) {
    switch (idx) {
        case ARPG_CFGSPRITE_PLAYER_LOOKDOWN_00:
            return ObjectPlayer_InitFromCfg(obj, &CFG_CHARACTER_PLAYER, position);
        case ARPG_CFGSPRITE_ENEMY_LOOKDOWN_00:
            return ObjectEnemy_InitFromCfg(obj, &CFG_CHARACTER_SKELETON_000_CHASE, position);
        default:
            return LOG_ERROR_M2V(M2ERR_INVALID_CFG_OBJTYP, Int32, idx);
    }
}

const m2::game_proxy impl::GAME_PROXY = {
        .entryUi = &ARPG_CFG_UI_ENTRYUI,
        .entryUiButtonHandler = ARPG_EntryUIButtonHandler,
        .pauseUi = &ARPG_CFG_UI_PAUSEUI,
        .pauseUiButtonHandler = ARPG_PauseUIButtonHandler,
        .tileSize = ARPG_CFG_TILE_SIZE,
        .textureMapFile = "resource/48.png",
        .textureMaskFile = "resource/48-Mask.png",
        .cfgSprites = ARPG_CFG_SPRITES,
        .cfgSpriteCount = ARPG_CFGSPRITE_N,
        .cfgHUDLeft = &CFG_UI_HUDLEFT,
        .cfgHUDRight = &CFG_UI_HUDRIGHT,
        .uiElementUpdateDynamic = UIElement_UpdateDynamic,
        .foregroundSpriteLoader = ARPG_ForegroundSpriteLoader
};
