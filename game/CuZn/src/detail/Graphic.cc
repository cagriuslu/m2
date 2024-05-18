#include <cuzn/detail/Graphic.h>
#include <m2g_ResourceType.pb.h>
#include <m2/Object.h>
#include <m2/Game.h>

void cuzn::draw_resources(m2::Character& chr) {
	auto coal_count = m2::iround(chr.get_resource(m2g::pb::COAL_CUBE_COUNT));
	auto iron_count = m2::iround(chr.get_resource(m2g::pb::IRON_CUBE_COUNT));
	auto beer_count = m2::iround(chr.get_resource(m2g::pb::BEER_BARREL_COUNT));

	auto nonzero_resource_type_count = (coal_count ? 1 : 0) + (iron_count ? 1 : 0) + (beer_count ? 1 : 0);
	if (nonzero_resource_type_count == 0) {
		return;
	}
	// Check if more than one resource is non-zero
	if (1 < nonzero_resource_type_count) {
		throw M2ERROR("Factory holds more than one type of resources");
	}

	const auto& pos = chr.parent().position;
	auto count = coal_count + iron_count + beer_count;
	auto count_sprite_type = static_cast<m2g::pb::SpriteType>(m2g::pb::TEXT_LABEL_1X - 1 + count);
	auto sprite_type = (coal_count ? m2g::pb::COAL_CUBE : (iron_count ? m2g::pb::IRON_CUBE : m2g::pb::BEER_BARREL));

	// Draw count
	m2::draw_real_2d(pos + m2::VecF{0.35f, 1.075f}, M2_GAME.get_sprite(count_sprite_type), m2::IsForegroundCompanion{false}, 0.0f);
	// Draw resource
	m2::draw_real_2d(pos + m2::VecF{1, 1}, M2_GAME.get_sprite(sprite_type), m2::IsForegroundCompanion{false}, 0.0f);
}
