#include <m2/object/tile.h>
#include <m2/object.hh>
#include "m2/Def.hh"
#include "m2/Box2DUtils.hh"
#include "m2/Game.hh"

std::pair<m2::object::Object&, ID> m2::object::tile::create(const vec2f& position, CfgSpriteIndex sprite_index) {
    auto obj_pair = object::create(position);

    const CfgSprite& sprite = GAME.proxy.cfgSprites[sprite_index];
    if (sprite.collider.type == CFG_COLLIDER_TYPE_RECTANGLE) {
        auto& phy = obj_pair.first.add_physique();
        phy.body = Box2DUtils_CreateStaticBox(obj_pair.first.physique_id, position, CATEGORY_STATIC_CLIFF, sprite.collider.params.rect.dims_m);
    }

    auto& gfx = obj_pair.first.add_terrain_graphic();
    gfx.textureRect = sprite.textureRect;

    return obj_pair;
}
