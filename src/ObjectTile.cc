#include <m2/object/Object.hh>
#include "m2/Def.hh"
#include "m2/Box2DUtils.hh"
#include "m2/Game.hh"

int ObjectTile_InitFromCfg(m2::object::Object* obj, CfgSpriteIndex spriteIndex, m2::vec2f position) {
	*obj = m2::object::Object{position};

	const CfgSprite *sprite = &GAME.proxy.cfgSprites[spriteIndex];

	if (sprite->collider.type == CFG_COLLIDER_TYPE_RECTANGLE) {
		auto& phy = obj->add_physique();
		phy.body = Box2DUtils_CreateStaticBox(obj->physique_id, position, CATEGORY_STATIC_CLIFF, sprite->collider.params.rect.dims_m);
	}

	auto& gfx = obj->add_terrain_graphic();
	gfx.textureRect = sprite->textureRect;

	return 0;
}
