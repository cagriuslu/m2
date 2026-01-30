#include <cuzn/detail/Graphic.h>
#include <m2/video/TextLabel.h>
#include <m2/Object.h>
#include <m2/Game.h>

void DrawResources(m2::Character& chr) {
	auto coal_count = chr.GetVariable(m2g::pb::COAL_CUBE_COUNT).GetIntOrZero();
	auto iron_count = chr.GetVariable(m2g::pb::IRON_CUBE_COUNT).GetIntOrZero();
	auto beer_count = chr.GetVariable(m2g::pb::BEER_BARREL_COUNT).GetIntOrZero();

	auto nonzero_resource_type_count = (coal_count ? 1 : 0) + (iron_count ? 1 : 0) + (beer_count ? 1 : 0);
	if (nonzero_resource_type_count == 0) {
		return;
	}
	// Check if more than one resource is non-zero
	if (1 < nonzero_resource_type_count) {
		throw M2_ERROR("Factory holds more than one type of resources");
	}

	const auto& pos = chr.Owner().InferPositionF();
	auto count = coal_count + iron_count + beer_count;
	auto count_sprite_type = static_cast<m2g::pb::SpriteType>(m2g::pb::TEXT_LABEL_1X - 1 + count);
	auto sprite_type = (coal_count ? m2g::pb::COAL_CUBE : (iron_count ? m2g::pb::IRON_CUBE : m2g::pb::BEER_BARREL));
	auto sprite_drop_shadow_type = (coal_count ? m2g::pb::COAL_CUBE_DROP_SHADOW : (iron_count ? m2g::pb::IRON_CUBE_DROP_SHADOW : m2g::pb::BEER_BARREL_DROP_SHADOW));
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(sprite_type));
	const auto& sprite_drop_shadow = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(sprite_drop_shadow_type));

	// Dim if necessary
	m2::Graphic::DimRenderingIfNecessary(chr.OwnerId(), M2_GAME.GetTextLabelCache().Texture());
	m2::Graphic::DimRenderingIfNecessary(chr.OwnerId(), sprite.GetTexture());
	// Draw count
	const auto& textLabel = std::get<m2::pb::TextLabel>(M2_GAME.GetSpriteOrTextLabel(count_sprite_type));
	const auto rect = M2_GAME.GetTextLabelCache().Create(textLabel.text(), m2::FontSizeOfTextLabel(textLabel));
	DrawTextLabelIn2dWorld(textLabel, rect, pos + m2::VecF{0.20f, 1.0f}, 0.0f);
	// Draw resource
	// We're not using a graphics component, so we have to draw the drop shadow ourselves.
	sprite_drop_shadow.DrawIn2dWorld(pos + m2::VecF{1.0f, 1.0f}, false, 0.0f);
	sprite.DrawIn2dWorld(pos + m2::VecF{1.0f, 1.0f}, false, 0.0f);
	// Undim
	m2::Graphic::UndimRendering(M2_GAME.GetTextLabelCache().Texture());
	m2::Graphic::UndimRendering(sprite.GetTexture());
}
