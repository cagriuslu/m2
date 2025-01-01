#include <m2/video/Sprite.h>
#include <m2/Math.h>
#include <m2/Game.h>

m2::Sprite::Sprite(const std::vector<SpriteSheet>& spriteSheets, const SpriteSheet& spriteSheet,
		SpriteEffectsSheet& spriteEffectsSheet, const pb::Sprite& sprite, const bool lightning)
		: _spriteSheet(&spriteSheet), _effectsSheet(&spriteEffectsSheet), _type(sprite.type()) {
	const pb::Sprite* original_sprite{};
	if (sprite.has_duplicate()) {
		// Lookup original sprite
		for (const auto& ss : spriteSheets) {
			for (const auto& s : ss.Pb().sprites()) {
				if (s.type() == sprite.duplicate().original_type()) {
					original_sprite = &s;
					break;  // Early out
				}
			}
			if (original_sprite) {
				break;  // Early out
			}
		}
		if (not original_sprite) {
			throw M2_ERROR("Unable to find original sprite");
		}
		_originalType = sprite.duplicate().original_type();
	} else {
		original_sprite = &sprite;
	}

	std::ranges::transform(original_sprite->regular().default_variant_draw_order(),
			std::back_inserter(_defaultVariantDrawOrder),
			[](int t) { return static_cast<pb::SpriteEffectType>(t); });
	_rect = RectI{original_sprite->regular().rect()};
	_originalRotationRad = original_sprite->regular().original_rotation() * PI;
	_ppm = original_sprite->regular().override_ppm() ? original_sprite->regular().override_ppm()
	                                                 : spriteSheet.Pb().ppm();
	_centerToOriginVecPx = VecF{original_sprite->regular().center_to_origin_vec_px()};
	_centerToOriginVecM = _centerToOriginVecPx / static_cast<float>(_ppm),
	_backgroundColliderType = original_sprite->regular().has_background_collider()
	    ? (original_sprite->regular().background_collider().has_rect_dims_px() ? box2d::ColliderType::RECTANGLE
	                                                                           : box2d::ColliderType::CIRCLE)
	    : box2d::ColliderType::NONE;
	_backgroundColliderOriginToOriginVecM =
	    VecF{original_sprite->regular().background_collider().origin_to_origin_vec_px()} / static_cast<float>(_ppm);

	_backgroundColliderRectDimsM =
	    VecF{original_sprite->regular().background_collider().rect_dims_px()} / static_cast<float>(_ppm);
	_backgroundColliderCircRadiusM =
	    original_sprite->regular().background_collider().circ_radius_px() / static_cast<float>(_ppm);
	_foregroundColliderType = original_sprite->regular().has_foreground_collider()
	    ? (original_sprite->regular().foreground_collider().has_rect_dims_px() ? box2d::ColliderType::RECTANGLE
	                                                                           : box2d::ColliderType::CIRCLE)
	    : box2d::ColliderType::NONE;
	_foregroundColliderOriginToOriginVecM =
	    VecF{original_sprite->regular().foreground_collider().origin_to_origin_vec_px()} / static_cast<float>(_ppm);
	_foregroundColliderRectDimsM =
	    VecF{original_sprite->regular().foreground_collider().rect_dims_px()} / static_cast<float>(_ppm);
	_foregroundColliderCircRadiusM =
	    original_sprite->regular().foreground_collider().circ_radius_px() / static_cast<float>(_ppm);
	_isBackgroundTile = original_sprite->regular().is_background_tile();

	// Fill named items
	for (const auto& named_item : original_sprite->regular().named_items()) {
		_namedItems.emplace_back(static_cast<m2g::pb::ItemType>(named_item));
	}
	if (sprite.has_duplicate()) {
		for (const auto& named_item : sprite.duplicate().additional_named_items()) {
			_namedItems.emplace_back(static_cast<m2g::pb::ItemType>(named_item));
		}
	}

	// Create foreground companion
	if (original_sprite->has_regular() && original_sprite->regular().foreground_companion_rects_size()) {
		_foregroundCompanionSpriteEffectsSheetRect = spriteEffectsSheet.create_foreground_companion_effect(
		    spriteSheet, original_sprite->regular().rect(), original_sprite->regular().foreground_companion_rects(),
		    lightning);
		_foregroundCompanionCenterToOriginVecPx =
		    VecF{original_sprite->regular().foreground_companion_center_to_origin_vec_px()};
		_foregroundCompanionCenterToOriginVecM =
		    VecF{original_sprite->regular().foreground_companion_center_to_origin_vec_px()} / static_cast<float>(_ppm);
	}

	// Create effects
	if (original_sprite->has_regular() && original_sprite->regular().effects_size()) {
		_effects.resize(pb::enum_value_count<pb::SpriteEffectType>());
		std::vector<bool> is_created(pb::enum_value_count<pb::SpriteEffectType>());
		for (const auto& effect : original_sprite->regular().effects()) {
			const auto index = enum_index(effect.type());
			// Check if the effect is already created
			if (is_created[index]) {
				throw M2_ERROR("Sprite has duplicate effect definition: " + std::to_string(effect.type()));
			}
			// Create effect
			switch (effect.type()) {
				case pb::SPRITE_EFFECT_MASK:
					_effects[index] = spriteEffectsSheet.create_mask_effect(
					    spriteSheet, original_sprite->regular().rect(), effect.mask_color(), lightning);
					break;
				case pb::SPRITE_EFFECT_GRAYSCALE:
					_effects[index] = spriteEffectsSheet.create_grayscale_effect(
					    spriteSheet, original_sprite->regular().rect(), lightning);
					break;
				case pb::SPRITE_EFFECT_IMAGE_ADJUSTMENT:
					_effects[index] = spriteEffectsSheet.create_image_adjustment_effect(
					    spriteSheet, original_sprite->regular().rect(), effect.image_adjustment(), lightning);
					break;
				case pb::SPRITE_EFFECT_BLURRED_DROP_SHADOW:
					_effects[index] = spriteEffectsSheet.create_blurred_drop_shadow_effect(spriteSheet, original_sprite->regular().rect(), effect.blurred_drop_shadow(), lightning);
					break;
				default:
					throw M2_ERROR(
					    "Encountered a sprite with unknown sprite effect: " + std::to_string(original_sprite->type()));
			}
			is_created[index] = true;
		}
	}
}

m2::Sprite::Sprite(SDL_Renderer* renderer, TTF_Font* font, const int fontSize, const pb::TextLabel& textLabel)
		: _type(textLabel.type()) {
	_textTexture = m2_move_or_throw_error(sdl::TextTexture::create_nowrap(renderer, font, fontSize, textLabel.text()));
	const auto dims = _textTexture->texture_dimensions();
	_rect = RectI{0, 0, dims.x, dims.y};
	_ppm = I(roundf(F(_rect.h) / textLabel.height_m()));
	_isBackgroundTile = textLabel.is_background_tile();
	if (textLabel.pull_half_cell()) {
		_centerToOriginVecM = VecF{0.5f, 0.0f};
		_centerToOriginVecPx = _centerToOriginVecM * F(_ppm);
	}
}

SDL_Texture* m2::Sprite::Texture(const SpriteVariant spriteVariant) const {
	if (_textTexture) {
		return _textTexture->texture();
	}
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

float m2::Sprite::SheetToScreenPixelMultiplier() const {
	return M2_GAME.Dimensions().RealOutputPixelsPerMeter() / static_cast<float>(_ppm);
}
m2::VecF m2::Sprite::CenterToOriginSrcpx(const SpriteVariant spriteVariant) const {
	if (std::holds_alternative<ForegroundCompanion>(spriteVariant)) {
		return ForegroundCompanionCenterToOriginVecPx();
	}
	return OriginalRotationRadians() != 0.0f
			? CenterToOriginVecPx().rotate(OriginalRotationRadians())
			: CenterToOriginVecPx();
}

std::vector<m2::Sprite> m2::LoadSprites(const std::vector<SpriteSheet>& spriteSheets,
		const google::protobuf::RepeatedPtrField<pb::TextLabel>& textLabels, SpriteEffectsSheet& spriteEffectsSheet,
		SDL_Renderer* renderer, TTF_Font* font, const int fontSize, const bool lightning) {
	std::vector<Sprite> sprites_vector(pb::enum_value_count<m2g::pb::SpriteType>());
	std::vector<bool> is_loaded(pb::enum_value_count<m2g::pb::SpriteType>());

	// Load sprites
	for (const auto& spriteSheet : spriteSheets) {
		for (const auto& sprite : spriteSheet.Pb().sprites()) {
			const auto index = pb::enum_index(sprite.type());
			// Check if the sprite is already loaded
			if (is_loaded[index]) {
				throw M2_ERROR("Sprite has duplicate definition: " + std::to_string(sprite.type()));
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
			throw M2_ERROR("Sprite has duplicate definition: " + std::to_string(textLabel.type()));
		}
		// Load sprite
		sprites_vector[index] = Sprite{renderer, font, fontSize, textLabel};
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

std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> m2::ListLevelEditorObjectSprites(
    const std::filesystem::path& objects_path) {
	auto objects = pb::json_file_to_message<pb::Objects>(objects_path);
	if (!objects) {
		throw M2_ERROR(objects.error());
	}

	std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> object_sprite_map;
	std::vector<bool> has_encountered(pb::enum_value_count<m2g::pb::ObjectType>());

	// Visit every object
	for (const auto& object : objects->objects()) {
		const auto index = pb::enum_index(object.type());
		// Check if object type already exists
		if (has_encountered[index]) {
			throw M2_ERROR("Object has duplicate definition: " + std::to_string(object.type()));
		}
		has_encountered[index] = true;

		if (object.main_sprite()) {
			object_sprite_map[object.type()] = object.main_sprite();
		}
	}

	// Check if every object type is encountered
	for (int e = 0; e < pb::enum_value_count<m2g::pb::ObjectType>(); ++e) {
		if (!has_encountered[e]) {
			throw M2_ERROR("Object is not defined: " + pb::enum_name<m2g::pb::ObjectType>(e));
		}
	}

	return object_sprite_map;
}

m2::void_expected m2::MoveBackground(const int from, const int to, const std::string& level) {
	if (from < 0 || 3 < from || to < 0 || 3 < to) {
		return make_unexpected("Invalid layer");
	}

	auto lb = pb::json_file_to_message<pb::Level>(M2_GAME.levels_dir / level);
	m2_reflect_unexpected(lb);

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
