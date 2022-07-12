#include <m2g/SpriteBlueprint.h>
#include <m2/object/Tile.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include <m2/box2d/Utils.h>

std::pair<m2::Object&, m2::ID> m2::obj::create_tile(const Vec2f& position, m2::SpriteIndex sprite_index) {
    auto obj_pair = create_object(position);
	auto& tile = obj_pair.first;

    const auto& sprite = m2g::sprites[sprite_index];
    if (std::holds_alternative<m2::ColliderBlueprint::Rectangle>(sprite.collider.variant)) {
        auto& phy = tile.add_physique();
        phy.body = m2::box2d::create_static_box(*GAME.world, tile.physique_id(), position, false, m2::box2d::CAT_GND_OBSTACLE, std::get<m2::ColliderBlueprint::Rectangle>(sprite.collider.variant).dims_m);
    }

    auto& gfx = tile.add_terrain_graphic();
    gfx.textureRect = sprite.texture_rect;

    return obj_pair;
}
