#include "m2/Object.hh"
#include "m2/Def.hh"
#include "m2/Box2DUtils.hh"
#include "m2/Game.hh"

int ObjectTile_InitFromCfg(Object* obj, CfgSpriteIndex spriteIndex, m2::vec2f position) {
	M2ERR_REFLECT(Object_Init(obj, position));

	const CfgSprite *sprite = &GAME.proxy.cfgSprites[spriteIndex];

	if (sprite->collider.type == CFG_COLLIDER_TYPE_RECTANGLE) {
		ComponentPhysique *phy = Object_AddPhysique(obj);
		phy->body = Box2DUtils_CreateStaticBox(GAME.physics.get_id(phy), position, CATEGORY_STATIC_CLIFF, sprite->collider.params.rect.dims_m);
	}

	ComponentGraphic *gfx = Object_AddTerrainGraphic(obj);
	gfx->textureRect = sprite->textureRect;

	return 0;
}
