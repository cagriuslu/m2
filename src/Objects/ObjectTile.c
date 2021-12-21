#include "../Object.h"
#include "../Log.h"
#include "../Box2DUtils.h"
#include "../Game.h"

int ObjectTile_InitFromCfg(Object* obj, const CfgGroundTexture *cfg, Vec2F position) {
	LOGFN_TRC();

	REFLECT_ERROR(Object_Init(obj, position, false));

	if (cfg->collider.type == CFG_COLLIDER_TYPE_RECTANGLE) {
		ComponentPhysics *phy = Object_AddPhysics(obj);
		phy->body = Box2DUtils_CreateStaticBox(Pool_GetId(&GAME->physics, phy), position, CATEGORY_STATIC_CLIFF, cfg->collider.colliderUnion.rect.dims_m);
	}

	ComponentGraphics *gfx = Object_AddTerrainGraphics(obj);
	gfx->txSrc = cfg->textureRect;

	return 0;
}
