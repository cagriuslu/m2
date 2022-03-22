#ifndef M2_GAMEPROXY_HH
#define M2_GAMEPROXY_HH

#include <m2/Cfg.hh>
#include <m2/object/Object.hh>

namespace m2 {
    struct game_proxy {
        const CfgUI *entryUi;

        M2Err (*entryUiButtonHandler)(CfgUIButtonType);

        const CfgUI *pauseUi;

        M2Err (*pauseUiButtonHandler)(CfgUIButtonType);

        unsigned tileSize;
        const char *textureMapFile;
        const char *textureMaskFile;

        const CfgSprite *cfgSprites;
        unsigned cfgSpriteCount;
        const CfgUI *cfgHUDLeft;
        const CfgUI *cfgHUDRight;

        M2Err (*uiElementUpdateDynamic)(UIElementState *state);

        M2Err (*foregroundSpriteLoader)(m2::object::Object *, CfgSpriteIndex, m2::vec2f);

        M2Err activate() const;
        M2Err exec_entry_ui() const;
        M2Err exec_pause_ui() const;
    };
}

#endif //M2_GAMEPROXY_HH
