#include <m2/video/Sprite.h>
#include <m2/Math.h>
#include <m2/Game.h>
#include <m2/Log.h>

m2::Sprite::Sprite(const std::vector<SpriteSheet>& spriteSheets, const SpriteSheet& spriteSheet,
		SpriteEffectsSheet& spriteEffectsSheet, const pb::Sprite& sprite)
		: _spriteSheet(&spriteSheet), _effectsSheet(&spriteEffectsSheet), _pb(&sprite) {
	// Lookup the original sprite
	if (sprite.has_duplicate()) {
		const pb::Sprite* originalPb{};
		for (const auto& ss : spriteSheets) {
			for (const auto& s : ss.Pb().sprites()) {
				if (s.type() == sprite.duplicate().original_type()) {
					originalPb = &s;
					break;  // Early out
				}
			}
			if (originalPb) { break; }
		}
		if (not originalPb) {
			throw M2_ERROR("Unable to find the original sprite");
		}
		_originalPb = originalPb;
	} else {
		_originalPb = &sprite;
	}

	if (sprite.has_duplicate() && sprite.duplicate().additional_effects_size()) {
		// TODO implement multiple effects
		if (const auto& effect = sprite.duplicate().additional_effects(0); effect.has_color_mask()) {
			_rect = spriteEffectsSheet.create_mask_effect(spriteSheet, _originalPb->regular().rect(), effect.color_mask());
			LOG_DEBUG("Created color mask effect for sprite", sprite.type(), _rect);
		} else if (effect.grayscale()) {
			_rect = spriteEffectsSheet.create_grayscale_effect(spriteSheet, _originalPb->regular().rect());
			LOG_DEBUG("Created grayscale effect for sprite", sprite.type(), _rect);
		} else if (effect.has_image_adjustment()) {
			_rect = spriteEffectsSheet.create_image_adjustment_effect(spriteSheet, _originalPb->regular().rect(), effect.image_adjustment());
			LOG_DEBUG("Created image adjustment effect for sprite", sprite.type(), _rect);
		} else if (effect.has_blurred_drop_shadow()) {
			_rect = spriteEffectsSheet.create_blurred_drop_shadow_effect(spriteSheet, _originalPb->regular().rect(), effect.blurred_drop_shadow());
			LOG_DEBUG("Created blurred drop shadow effect for sprite", sprite.type(), _rect);
		} else {
			throw M2_ERROR("Missing or unimplemented sprite effect");
		}
	} else {
		_rect = RectI{_originalPb->regular().rect()};
	}
	_originalRotationRad = _originalPb->regular().original_rotation() * PI;
	_ppm = _originalPb->regular().override_ppm() ? _originalPb->regular().override_ppm() : spriteSheet.Pb().ppm();
	_centerToOriginVecPx = VecF{_originalPb->regular().center_to_origin_vec_px()};
	_centerToOriginVecM = _centerToOriginVecPx / static_cast<float>(_ppm);

	// Fill named cards
	for (const auto& named_card : _originalPb->regular().named_cards()) {
		_namedCards.emplace_back(static_cast<m2g::pb::CardType>(named_card));
	}
	if (sprite.has_duplicate()) {
		for (const auto& named_card : sprite.duplicate().additional_named_cards()) {
			_namedCards.emplace_back(static_cast<m2g::pb::CardType>(named_card));
		}
	}

	// Create foreground companion
	if (_originalPb->has_regular() && _originalPb->regular().has_foreground_companion()) {
		_foregroundCompanionSpriteEffectsSheetRect = spriteEffectsSheet.create_foreground_companion_effect(
		    spriteSheet, _originalPb->regular().rect(),
		    _originalPb->regular().foreground_companion().rects());
		_foregroundCompanionCenterToOriginVecPx =
		    VecF{_originalPb->regular().foreground_companion().center_to_origin_vec_px()};
		_foregroundCompanionCenterToOriginVecM =
		    VecF{_originalPb->regular().foreground_companion().center_to_origin_vec_px()} / static_cast<float>(_ppm);
	}
}

SDL_Texture* m2::Sprite::GetTexture(const bool foregroundCompanion) const {
	if (foregroundCompanion) {
		return _effectsSheet->Texture();
	}
	if (_pb->has_duplicate() && _pb->duplicate().additional_effects_size()) {
		return _effectsSheet->Texture();
	}
	return Sheet().texture();
}

m2::VecF m2::Sprite::TextureTotalDims(const bool foregroundCompanion) const {
	if (foregroundCompanion) {
		return {_effectsSheet->texture_width(), _effectsSheet->texture_height()};
	}
	if (_pb->has_duplicate() && _pb->duplicate().additional_effects_size()) {
		return {_effectsSheet->texture_width(), _effectsSheet->texture_height()};
	}
	return {Sheet().surface()->w, Sheet().surface()->h};
}

const m2::RectI& m2::Sprite::GetRect(const bool foregroundCompanion) const {
	if (foregroundCompanion) {
		return *_foregroundCompanionSpriteEffectsSheetRect;
	}
	return _rect;
}

float m2::Sprite::SourceToOutputPixelMultiplier() const {
	return M2_GAME.Dimensions().OutputPixelsPerMeter() / static_cast<float>(_ppm);
}

m2::VecF m2::Sprite::CenterToOriginVecSrcpx(const bool foregroundCompanion) const {
	if (foregroundCompanion) {
		return ForegroundCompanionCenterToOriginVecPx();
	}
	return OriginalRotationRadians() != 0.0f
			? CenterToOriginVecPx().Rotate(OriginalRotationRadians())
			: CenterToOriginVecPx();
}

m2::VecF m2::Sprite::ScreenOriginToCenterVecOutpx(const VecF& position, const bool foregroundCompanion) const {
	return ScreenOriginToPositionVecPx(position) - CenterToOriginVecOutpx(foregroundCompanion);
}

void m2::Sprite::DrawIn2dWorld(const VecF& position, const bool foregroundCompanion, const float angle, MAYBE bool is_foreground, MAYBE float z) const {
	const auto sourceRect = static_cast<SDL_Rect>(GetRect(foregroundCompanion));
	DrawTextureIn2dWorld(
			M2_GAME.renderer,
			GetTexture(foregroundCompanion),
			&sourceRect,
			OriginalRotationRadians(),
			M2_GAME.Dimensions().OutputPixelsPerMeter() / ToFloat(Ppm()),
			CenterToOriginVecOutpx(foregroundCompanion),
			ScreenOriginToCenterVecOutpx(position, foregroundCompanion),
			angle
	);
}
void m2::Sprite::DrawIn3dWorld(const VecF& position, const bool foregroundCompanion, const float angle, const bool is_foreground, const float z) const {
	const auto sourceRect = static_cast<SDL_Rect>(GetRect(foregroundCompanion));
	DrawTextureIn3dWorld(
			M2_GAME.renderer,
			GetTexture(foregroundCompanion),
			&sourceRect,
			ToFloat(Ppm()),
			CenterToOriginVecOutpx(foregroundCompanion),
			OriginalRotationRadians(),
			TextureTotalDims(foregroundCompanion),
			position,
			z,
			angle,
			is_foreground);
}

bool m2::IsSpriteBackgroundTile(const std::variant<Sprite,pb::TextLabel>& spriteOrTextLabel) {
	if (std::holds_alternative<Sprite>(spriteOrTextLabel)) {
		return std::get<Sprite>(spriteOrTextLabel).IsBackgroundTile();
	}
	return std::get<pb::TextLabel>(spriteOrTextLabel).is_background_tile();
}

m2g::pb::SpriteType m2::ToSpriteType(const std::variant<Sprite,pb::TextLabel>& spriteOrTextLabel) {
	if (std::holds_alternative<Sprite>(spriteOrTextLabel)) {
		return std::get<Sprite>(spriteOrTextLabel).Type();
	}
	return std::get<pb::TextLabel>(spriteOrTextLabel).type();
}

std::vector<std::variant<m2::Sprite, m2::pb::TextLabel>> m2::LoadSprites(const std::vector<SpriteSheet>& spriteSheets,
		const google::protobuf::RepeatedPtrField<pb::TextLabel>& textLabels, SpriteEffectsSheet& spriteEffectsSheet) {
	std::vector<std::variant<Sprite, pb::TextLabel>> sprites_vector(pb::enum_value_count<m2g::pb::SpriteType>());
	std::vector<bool> is_loaded(pb::enum_value_count<m2g::pb::SpriteType>());

	// Load sprites
	for (const auto& spriteSheet : spriteSheets) {
		for (const auto& sprite : spriteSheet.Pb().sprites()) {
			const auto index = pb::enum_index(sprite.type());
			// Check if the sprite is already loaded
			if (is_loaded[index]) {
				throw M2_ERROR("Sprite has duplicate definition: " + m2::ToString(sprite.type()));
			}
			// Load sprite
			sprites_vector[index] = Sprite{spriteSheets, spriteSheet, spriteEffectsSheet, sprite};
			is_loaded[index] = true;
		}
	}
	for (const auto& textLabel : textLabels) {
		const auto index = pb::enum_index(textLabel.type());
		// Check if the sprite is already loaded
		if (is_loaded[index]) {
			throw M2_ERROR("Sprite has duplicate definition: " + m2::ToString(textLabel.type()));
		}
		// Load sprite
		sprites_vector[index] = textLabel;
		is_loaded[index] = true;
	}

	// Check if every sprite type is loaded
	for (int e = 0; e < pb::enum_value_count<m2g::pb::SpriteType>(); ++e) {
		if (!is_loaded[e]) {
			throw M2_ERROR("Sprite is not defined: " + pb::enum_name<m2g::pb::SpriteType>(e));
		}
	}

	return sprites_vector;
}

m2::void_expected m2::MoveBackground(const int from, const int to, const std::string& level) {
	if (from < 0 || 3 < from || to < 0 || 3 < to) {
		return make_unexpected("Invalid layer");
	}

	auto lb = pb::json_file_to_message<pb::Level>(M2_GAME.GetResources().GetLevelsDir() / level);
	m2ReflectUnexpected(lb);

	// Ensure there are enough layers
	while (lb->background_layers_size() < 4) {
		lb->add_background_layers();
	}

	// Move
	const auto layer = lb->background_layers(from);
	lb->mutable_background_layers(to)->CopyFrom(layer);
	lb->mutable_background_layers(from)->Clear();

	// Save
	return message_to_json_file(*lb, M2_GAME.GetResources().GetLevelsDir() / level);
}
