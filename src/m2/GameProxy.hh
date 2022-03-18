#ifndef M2_GAMEPROXY_HH
#define M2_GAMEPROXY_HH

#include <m2/Cfg.hh>
#include <m2/Object.hh>

typedef struct _GameProxy {
	const CfgUI* entryUi;
	M2Err (*entryUiButtonHandler)(CfgUIButtonType);
	const CfgUI* pauseUi;
	M2Err (*pauseUiButtonHandler)(CfgUIButtonType);

	unsigned tileSize;
	const char *textureMapFile;
	const char *textureMaskFile;

	const CfgSprite *cfgSprites;
	unsigned cfgSpriteCount;
	const CfgUI *cfgHUDLeft;
	const CfgUI *cfgHUDRight;
	M2Err (*uiElementUpdateDynamic)(UIElementState *state);

	size_t componentDefenseDataSize;
	size_t componentOffenseDataSize;
	M2Err (*foregroundSpriteLoader)(Object*, CfgSpriteIndex, m2::vec2f);

	void (*destructor)(struct _GameProxy*);
} GameProxy;

M2Err GameProxy_Init(GameProxy *gp);
M2Err GameProxy_InitARPG(GameProxy *gp);

// Call after initializing the proxy
M2Err GameProxy_Activate(GameProxy *gp);

M2Err GameProxy_ExecuteEntryUI(GameProxy *gp);
M2Err GameProxy_ExecutePauseUI(GameProxy *gp);

void GameProxy_Term(GameProxy *gp);

#endif //M2_GAMEPROXY_HH
