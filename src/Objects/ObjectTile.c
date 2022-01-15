#include "../Object.h"
#include "../Def.h"
#include "../Box2DUtils.h"
#include "../Game.h"

int ObjectTile_InitFromCfg(Object* obj, const CfgGroundTexture *cfg, Vec2F position) {
	LOGFN_TRC();

	XERR_REFLECT(Object_Init(obj, position, false));

	if (cfg->collider.type == CFG_COLLIDER_TYPE_RECTANGLE) {
		ComponentPhysique *phy = Object_AddPhysique(obj);
		phy->body = Box2DUtils_CreateStaticBox(Pool_GetId(&GAME->physics, phy), position, CATEGORY_STATIC_CLIFF, cfg->collider.colliderUnion.rect.dims_m);
	}

	ComponentGraphic *gfx = Object_AddTerrainGraphic(obj);
	gfx->textureRect = cfg->textureRect;

	return 0;
}
