#include <m2/video/Sprite.h>
#include <m2/Math.h>
#include <m2/Game.h>
#include <m2/Log.h>

m2::Sprite::Sprite(const std::vector<SpriteSheet>& spriteSheets, const SpriteSheet& spriteSheet,
		SpriteEffectsSheet& spriteEffectsSheet, const pb::Sprite& sprite, const bool lightning)
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
			if (originalPb) {
				break;  // Early out
			}
		}
		if (not originalPb) {
			throw M2_ERROR("Unable to find the original sprite");
		}
		_originalPb = originalPb;
	} else {
		_originalPb = &sprite;
	}

	std::ranges::transform(_originalPb->regular().default_variant_draw_order(),
			std::back_inserter(_defaultVariantDrawOrder),
			[](int t) { return static_cast<pb::SpriteEffectType>(t); });
	_rect = RectI{_originalPb->regular().rect()};
	_originalRotationRad = _originalPb->regular().original_rotation() * PI;
	_ppm = _originalPb->regular().override_ppm() ? _originalPb->regular().override_ppm() : spriteSheet.Pb().ppm();
	_centerToOriginVecPx = VecF{_originalPb->regular().center_to_origin_vec_px()};
	_centerToOriginVecM = _centerToOriginVecPx / static_cast<float>(_ppm);

	// Fill named items
	for (const auto& named_item : _originalPb->regular().named_items()) {
		_namedItems.emplace_back(static_cast<m2g::pb::ItemType>(named_item));
	}
	if (sprite.has_duplicate()) {
		for (const auto& named_item : sprite.duplicate().additional_named_items()) {
			_namedItems.emplace_back(static_cast<m2g::pb::ItemType>(named_item));
		}
	}

	// Create foreground companion
	if (_originalPb->has_regular() && _originalPb->regular().has_foreground_companion()) {
		_foregroundCompanionSpriteEffectsSheetRect = spriteEffectsSheet.create_foreground_companion_effect(
		    spriteSheet, _originalPb->regular().rect(),
		    _originalPb->regular().foreground_companion().foreground_companion_rects(), lightning);
		_foregroundCompanionCenterToOriginVecPx =
		    VecF{_originalPb->regular().foreground_companion().center_to_origin_vec_px()};
		_foregroundCompanionCenterToOriginVecM =
		    VecF{_originalPb->regular().foreground_companion().center_to_origin_vec_px()} / static_cast<float>(_ppm);
	}

	// Create effects
	if (_originalPb->has_regular() && _originalPb->regular().effects_size()) {
		_effects.resize(pb::enum_value_count<pb::SpriteEffectType>());
		std::vector<bool> is_created(pb::enum_value_count<pb::SpriteEffectType>());
		for (const auto& effect : _originalPb->regular().effects()) {
			const auto index = enum_index(effect.type());
			// Check if the effect is already created
			if (is_created[index]) {
				throw M2_ERROR("Sprite has duplicate effect definition: " + m2::ToString(effect.type()));
			}
			// Create effect
			switch (effect.type()) {
				case pb::SPRITE_EFFECT_MASK:
					_effects[index] = spriteEffectsSheet.create_mask_effect(
					    spriteSheet, _originalPb->regular().rect(), effect.mask_color(), lightning);
					LOG_DEBUG("Sprite mask effect rect", _originalPb->type(), _effects[index]);
					break;
				case pb::SPRITE_EFFECT_GRAYSCALE:
					_effects[index] = spriteEffectsSheet.create_grayscale_effect(
					    spriteSheet, _originalPb->regular().rect(), lightning);
					LOG_DEBUG("Grayscale effect rect", _originalPb->type(), _effects[index]);
					break;
				case pb::SPRITE_EFFECT_IMAGE_ADJUSTMENT:
					_effects[index] = spriteEffectsSheet.create_image_adjustment_effect(
					    spriteSheet, _originalPb->regular().rect(), effect.image_adjustment(), lightning);
					LOG_DEBUG("Image adjustment effect rect", _originalPb->type(), _effects[index]);
					break;
				case pb::SPRITE_EFFECT_BLURRED_DROP_SHADOW:
					_effects[index] = spriteEffectsSheet.create_blurred_drop_shadow_effect(spriteSheet, _originalPb->regular().rect(), effect.blurred_drop_shadow(), lightning);
					LOG_DEBUG("Blurred drop shadow effect rect", _originalPb->type(), _effects[index]);
					break;
				default:
					throw M2_ERROR(
					    "Encountered a sprite with unknown sprite effect: " + m2::ToString(_originalPb->type()));
			}
			is_created[index] = true;
		}
	}
}

SDL_Texture* m2::Sprite::Texture(const SpriteVariant spriteVariant) const {
	if (std::holds_alternative<DefaultVariant>(spriteVariant)) {
		return Sheet().texture();
	}
	return EffectsTexture();
}

m2::VecF m2::Sprite::TextureTotalDims(const SpriteVariant spriteVariant) const {
	if (std::holds_alternative<DefaultVariant>(spriteVariant)) {
		return {Sheet().surface()->w, Sheet().surface()->h};
	}
	return {EffectsSheet()->texture_width(), EffectsSheet()->texture_height()};
}

const m2::RectI& m2::Sprite::Rect(const SpriteVariant spriteVariant) const {
	if (std::holds_alternative<DefaultVariant>(spriteVariant)) {
		return Rect();
	}
	if (std::holds_alternative<ForegroundCompanion>(spriteVariant)) {
		return *_foregroundCompanionSpriteEffectsSheetRect;
	}
	return EffectRect(std::get<pb::SpriteEffectType>(spriteVariant));
}

float m2::Sprite::SourceToOutputPixelMultiplier() const {
	return M2_GAME.Dimensions().OutputPixelsPerMeter() / static_cast<float>(_ppm);
}

m2::VecF m2::Sprite::CenterToOriginVecSrcpx(const SpriteVariant spriteVariant) const {
	if (std::holds_alternative<ForegroundCompanion>(spriteVariant)) {
		return ForegroundCompanionCenterToOriginVecPx();
	}
	return OriginalRotationRadians() != 0.0f
			? CenterToOriginVecPx().Rotate(OriginalRotationRadians())
			: CenterToOriginVecPx();
}

m2::VecF m2::Sprite::ScreenOriginToCenterVecOutpx(const VecF& position, const SpriteVariant sprite_variant) const {
	return ScreenOriginToPositionVecPx(position) - CenterToOriginVecOutpx(sprite_variant);
}

void m2::Sprite::DrawIn2dWorld(const VecF& position, const SpriteVariant sprite_variant, const float angle, MAYBE bool is_foreground, MAYBE float z) const {
	const auto sourceRect = static_cast<SDL_Rect>(Rect(sprite_variant));
	DrawTextureIn2dWorld(
			M2_GAME.renderer,
			Texture(sprite_variant),
			&sourceRect,
			OriginalRotationRadians(),
			M2_GAME.Dimensions().OutputPixelsPerMeter() / F(Ppm()),
			CenterToOriginVecOutpx(sprite_variant),
			ScreenOriginToCenterVecOutpx(position, sprite_variant),
			angle
	);
}
void m2::Sprite::DrawIn3dWorld(const VecF& position, const SpriteVariant sprite_variant, const float angle, const bool is_foreground, const float z) const {
	const auto sourceRect = static_cast<SDL_Rect>(Rect(sprite_variant));
	DrawTextureIn3dWorld(
			M2_GAME.renderer,
			Texture(sprite_variant),
			&sourceRect,
			F(Ppm()),
			CenterToOriginVecOutpx(sprite_variant),
			OriginalRotationRadians(),
			TextureTotalDims(sprite_variant),
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
		const google::protobuf::RepeatedPtrField<pb::TextLabel>& textLabels, SpriteEffectsSheet& spriteEffectsSheet,
		const bool lightning) {
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
			sprites_vector[index] = Sprite{spriteSheets, spriteSheet, spriteEffectsSheet, sprite, lightning};
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

	auto lb = pb::json_file_to_message<pb::Level>(M2_GAME.levels_dir / level);
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
	return message_to_json_file(*lb, M2_GAME.levels_dir / level);
}
