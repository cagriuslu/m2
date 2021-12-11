#include "../Object.h"
#include "../Log.h"
#include "../Box2DUtils.h"

int ObjectTile_InitFromCfg(Object* obj, const CfgGroundTile *cfg, Vec2F position) {
	LOGFN_TRC();

	REFLECT_ERROR(Object_Init(obj, position, false));

	if (cfg->collider.type == COLLIDER_TYPE_RECTANGLE) {
		ID phyId = 0;
		ComponentPhysics *phy = Object_AddPhysics(obj, &phyId);
		phy->body = Box2DUtils_CreateStaticBox(phyId, position, CATEGORY_STATIC_CLIFF, cfg->collider.colliderUnion.rect.dims_m);
	}

	ComponentGraphics *gfx = Object_AddTerrainGraphics(obj, NULL);
	gfx->txSrc = cfg->textureRect;

	return 0;
}
