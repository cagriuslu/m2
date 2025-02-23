#include <cuzn/detail/Graphic.h>
#include <m2g_ResourceType.pb.h>
#include <m2/video/TextLabel.h>
#include <m2/Object.h>
#include <m2/Game.h>

void DrawResources(m2::Character& chr) {
	auto coal_count = m2::iround(chr.GetResource(m2g::pb::COAL_CUBE_COUNT));
	auto iron_count = m2::iround(chr.GetResource(m2g::pb::IRON_CUBE_COUNT));
	auto beer_count = m2::iround(chr.GetResource(m2g::pb::BEER_BARREL_COUNT));

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
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(sprite_type));

	// Dim if necessary
	m2::Graphic::DimRenderingIfNecessary(chr.owner_id(), M2_GAME.TextLabelCache().Texture());
	m2::Graphic::DimRenderingIfNecessary(chr.owner_id(), sprite.Texture());
	// Draw count
	const auto& textLabel = std::get<m2::pb::TextLabel>(M2_GAME.GetSpriteOrTextLabel(count_sprite_type));
	const auto rect = M2_GAME.TextLabelCache().Create(textLabel.text(), m2::FontSizeOfTextLabel(textLabel));
	DrawTextLabelIn2dWorld(textLabel, rect, pos + m2::VecF{0.20f, 1.0f}, 0.0f);
	// Draw resource
	// We're not using a graphics component, so we have to draw the drop shadow ourselves.
	sprite.DrawIn2dWorld(pos + m2::VecF{1.0f, 1.05f}, m2::pb::SpriteEffectType{m2::pb::SPRITE_EFFECT_BLURRED_DROP_SHADOW}, 0.0f);
	sprite.DrawIn2dWorld(pos + m2::VecF{1.0f, 1.0f}, {}, 0.0f);
	// Undim
	m2::Graphic::UndimRendering(M2_GAME.TextLabelCache().Texture());
	m2::Graphic::UndimRendering(sprite.Texture());
}
