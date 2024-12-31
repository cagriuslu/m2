#include <cuzn/detail/Graphic.h>
#include <m2g_ResourceType.pb.h>
#include <m2/Object.h>
#include <m2/Game.h>

void DrawResources(m2::Character& chr) {
	auto coal_count = m2::iround(chr.get_resource(m2g::pb::COAL_CUBE_COUNT));
	auto iron_count = m2::iround(chr.get_resource(m2g::pb::IRON_CUBE_COUNT));
	auto beer_count = m2::iround(chr.get_resource(m2g::pb::BEER_BARREL_COUNT));

	auto nonzero_resource_type_count = (coal_count ? 1 : 0) + (iron_count ? 1 : 0) + (beer_count ? 1 : 0);
	if (nonzero_resource_type_count == 0) {
		return;
	}
	// Check if more than one resource is non-zero
	if (1 < nonzero_resource_type_count) {
		throw M2_ERROR("Factory holds more than one type of resources");
	}

	const auto& pos = chr.owner().position;
	auto count = coal_count + iron_count + beer_count;
	auto count_sprite_type = static_cast<m2g::pb::SpriteType>(m2g::pb::TEXT_LABEL_1X - 1 + count);
	auto sprite_type = (coal_count ? m2g::pb::COAL_CUBE : (iron_count ? m2g::pb::IRON_CUBE : m2g::pb::BEER_BARREL));

	// Dim if necessary
	m2::Graphic::dim_rendering_if_necessary(chr.owner_id(), M2_GAME.GetSprite(count_sprite_type).Texture());
	m2::Graphic::dim_rendering_if_necessary(chr.owner_id(), M2_GAME.GetSprite(sprite_type).Texture());
	// Draw count
	m2::draw_real_2d(pos + m2::VecF{0.20f, 1.06f}, M2_GAME.GetSprite(count_sprite_type), {}, 0.0f);
	// Draw resource
	// We're not using a graphics component, so we have to draw the drop shadow ourselves.
	m2::draw_real_2d(pos + m2::VecF{1.0f, 1.05f}, M2_GAME.GetSprite(sprite_type), m2::pb::SpriteEffectType{m2::pb::SPRITE_EFFECT_BLURRED_DROP_SHADOW}, 0.0f);
	m2::draw_real_2d(pos + m2::VecF{1.0f, 1.0f}, M2_GAME.GetSprite(sprite_type), {}, 0.0f);
	// Undim
	m2::Graphic::undim_rendering(M2_GAME.GetSprite(count_sprite_type).Texture());
	m2::Graphic::undim_rendering(M2_GAME.GetSprite(sprite_type).Texture());
}
