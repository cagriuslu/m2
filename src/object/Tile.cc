#include <m2g/Object.h>
#include <m2/sdl/Utils.hh>
#include <m2/object/Tile.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include <m2/box2d/Utils.h>

std::pair<m2::Object&, m2::Id> m2::obj::create_tile(const Vec2f& position, const m2::Sprite& sprite) {
    auto obj_pair = create_object(position);
	obj_pair.first.add_terrain_graphic(sprite);

	if (sprite.sprite().has_background_collider()) {
		auto collider_obj_pair = create_object(position + sprite.background_collider_center_offset_m());
		auto& collider = sprite.sprite().background_collider();
		if (collider.has_rect_dims_px()) {
			auto& phy = collider_obj_pair.first.add_physique();
			m2::pb::BodyBlueprint bp;
			bp.set_type(m2::pb::BodyType::STATIC);
			bp.set_allow_sleep(true);
			bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_w(collider.rect_dims_px().w() / (float)sprite.ppm());
			bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_h(collider.rect_dims_px().h() / (float)sprite.ppm());
			bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);
			phy.body = m2::box2d::create_body(*GAME.world, collider_obj_pair.first.physique_id(), collider_obj_pair.first.position, bp);
		} else {
			throw M2FATAL("Circular tile collider unimplemented");
		}
	}

	// Use foreground collider as a secondary background collider
	if (sprite.sprite().has_foreground_collider()) {
		auto collider_obj_pair = create_object(position + sprite.foreground_collider_center_offset_m());
		auto& collider = sprite.sprite().foreground_collider();
		if (collider.has_rect_dims_px()) {
			auto& phy = collider_obj_pair.first.add_physique();
			m2::pb::BodyBlueprint bp;
			bp.set_type(m2::pb::BodyType::STATIC);
			bp.set_allow_sleep(true);
			bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_w(collider.rect_dims_px().w() / (float)sprite.ppm());
			bp.mutable_background_fixture()->mutable_rect()->mutable_dims()->set_h(collider.rect_dims_px().h() / (float)sprite.ppm());
			bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);
			phy.body = m2::box2d::create_body(*GAME.world, collider_obj_pair.first.physique_id(), collider_obj_pair.first.position, bp);
		} else {
			throw M2FATAL("Circular tile collider unimplemented");
		}
	}

	if (sprite.has_foreground_companion()) {
		obj::create_tile_foreground_companion(position, sprite);
	}

    return obj_pair;
}

std::pair<m2::Object&, m2::Id> m2::obj::create_tile_foreground_companion(const Vec2f& position, const m2::Sprite& sprite) {
	auto obj_pair = create_object(position + sprite.foreground_companion_center_offset_m());
	auto& companion = obj_pair.first;

	auto& gfx = companion.add_graphic(sprite);
	gfx.draw_foreground_companion = true;

	return obj_pair;
}
