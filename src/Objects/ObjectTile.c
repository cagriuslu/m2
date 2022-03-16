#include "m2/Object.h"
#include "m2/Def.h"
#include "m2/Box2DUtils.h"
#include "m2/Game.h"

int ObjectTile_InitFromCfg(Object* obj, CfgSpriteIndex spriteIndex, Vec2F position) {
	M2ERR_REFLECT(Object_Init(obj, position));

	const CfgSprite *sprite = &GAME->proxy.cfgSprites[spriteIndex];

	if (sprite->collider.type == CFG_COLLIDER_TYPE_RECTANGLE) {
		ComponentPhysique *phy = Object_AddPhysique(obj);
		phy->body = Box2DUtils_CreateStaticBox(Pool_GetId(&GAME->physics, phy), position, CATEGORY_STATIC_CLIFF, sprite->collider.params.rect.dims_m);
	}

	ComponentGraphic *gfx = Object_AddTerrainGraphic(obj);
	gfx->textureRect = sprite->textureRect;

	return 0;
}
