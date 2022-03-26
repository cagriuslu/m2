#include <m2/object/Tile.h>
#include <m2/Object.h>
#include "m2/Def.hh"
#include "m2/Game.hh"
#include <m2/box2d/Utils.h>

std::pair<m2::Object&, ID> m2::object::create_tile(const Vec2f& position, CfgSpriteIndex sprite_index) {
    auto obj_pair = create_object(position);

    const CfgSprite& sprite = GAME.proxy.cfgSprites[sprite_index];
    if (sprite.collider.type == CFG_COLLIDER_TYPE_RECTANGLE) {
        auto& phy = obj_pair.first.add_physique();
        phy.body = m2::box2d::create_static_box(*GAME.world, obj_pair.first.physique_id, position, false, m2::box2d::CATEGORY_STATIC_CLIFF, sprite.collider.params.rect.dims_m);
    }

    auto& gfx = obj_pair.first.add_terrain_graphic();
    gfx.textureRect = sprite.textureRect;

    return obj_pair;
}
