#include <impl/public/SpriteBlueprint.h>
#include <m2/object/Tile.h>
#include <m2/Object.h>
#include "m2/Def.h"
#include "m2/Game.hh"
#include <m2/box2d/Utils.h>

std::pair<m2::Object&, ID> m2::object::create_tile(const Vec2f& position, m2::SpriteIndex sprite_index) {
    auto obj_pair = create_object(position);

    const auto& sprite = impl::sprites[sprite_index];
    if (std::holds_alternative<m2::ColliderBlueprint::Rectangle>(sprite.collider.variant)) {
        auto& phy = obj_pair.first.add_physique();
        phy.body = m2::box2d::create_static_box(*GAME.world, obj_pair.first.physique_id, position, false, m2::box2d::CATEGORY_STATIC_CLIFF, std::get<m2::ColliderBlueprint::Rectangle>(sprite.collider.variant).dims_m);
    }

    auto& gfx = obj_pair.first.add_terrain_graphic();
    gfx.textureRect = sprite.texture_rect;

    return obj_pair;
}
