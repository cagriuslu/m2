#pragma once
#include <m2/video/SpriteEffectSheet.h>
#include <m2/video/SpriteSheet.h>
#include <m2/video/SpriteVariant.h>
#include <m2/box2d/Body.h>
#include <m2/sdl/TextTexture.h>
#include <m2/protobuf/Detail.h>

namespace m2 {
	class Sprite final {
		const SpriteSheet* _spriteSheet{};
		const SpriteEffectsSheet* _effectsSheet{};

		m2g::pb::SpriteType _type{};
		std::optional<m2g::pb::SpriteType> _originalType;
		std::vector<RectI> _effects;
		std::vector<pb::SpriteEffectType> _defaultVariantDrawOrder;
		std::optional<RectI> _foregroundCompanionSpriteEffectsSheetRect;
		std::optional<VecF> _foregroundCompanionCenterToOriginVecPx;
		std::optional<VecF> _foregroundCompanionCenterToOriginVecM;
		RectI _rect;
		float _originalRotationRad{};
		int _ppm{};
		VecF _centerToOriginVecPx;
		VecF _centerToOriginVecM;
		box2d::ColliderType _backgroundColliderType{box2d::ColliderType::NONE};
		VecF _backgroundColliderOriginToOriginVecM;
		VecF _backgroundColliderRectDimsM;
		float _backgroundColliderCircRadiusM{};
		box2d::ColliderType _foregroundColliderType{box2d::ColliderType::NONE};
		VecF _foregroundColliderOriginToOriginVecM;
		VecF _foregroundColliderRectDimsM;
		float _foregroundColliderCircRadiusM{};
		bool _isBackgroundTile{};
		std::vector<m2g::pb::ItemType> _namedItems;

		// Text label
		std::optional<sdl::TextTexture> _textTexture;

	   public:
		Sprite() = default;
		Sprite(const std::vector<SpriteSheet>& spriteSheets, const SpriteSheet& spriteSheet,
				SpriteEffectsSheet& spriteEffectsSheet, const pb::Sprite& sprite, bool lightning);
		Sprite(SDL_Renderer* renderer, TTF_Font* font, int fontSize, const pb::TextLabel& textLabel);

		// Accessors
		[[nodiscard]] const SpriteSheet& Sheet() const { return *_spriteSheet; }
		[[nodiscard]] const SpriteEffectsSheet* EffectsSheet() const { return _effectsSheet; }
		[[nodiscard]] m2g::pb::SpriteType Type() const { return _type; }
		[[nodiscard]] std::optional<m2g::pb::SpriteType> OriginalType() const { return _originalType; }
		[[nodiscard]] SDL_Texture* EffectsTexture() const { return _effectsSheet ? _effectsSheet->Texture() : nullptr; }
		[[nodiscard]] const RectI& EffectRect(const pb::SpriteEffectType effect_type) const { return _effects[enum_index(effect_type)]; }
		[[nodiscard]] const std::vector<pb::SpriteEffectType>& DefaultVariantDrawOrder() const { return _defaultVariantDrawOrder; }
		[[nodiscard]] bool HasForegroundCompanion() const { return _foregroundCompanionCenterToOriginVecM.has_value(); }
		[[nodiscard]] VecF ForegroundCompanionCenterToOriginVecPx() const { return _foregroundCompanionCenterToOriginVecPx.value(); }
		[[nodiscard]] VecF ForegroundCompanionCenterToOriginVecM() const { return _foregroundCompanionCenterToOriginVecM.value(); }
		[[nodiscard]] const RectI& Rect() const { return _rect; }
		[[nodiscard]] float OriginalRotationRadians() const { return _originalRotationRad; }
		[[nodiscard]] int Ppm() const { return _ppm; }
		[[nodiscard]] const VecF& CenterToOriginVecPx() const { return _centerToOriginVecPx; }
		[[nodiscard]] const VecF& CenterToOriginVecM() const { return _centerToOriginVecM; }
		[[nodiscard]] box2d::ColliderType BackgroundColliderType() const { return _backgroundColliderType; }
		[[nodiscard]] VecF BackgroundColliderOriginToOriginVecM() const { return _backgroundColliderOriginToOriginVecM; }
		[[nodiscard]] VecF BackgroundColliderRectDimsM() const { return _backgroundColliderRectDimsM; }
		[[nodiscard]] float BackgroundColliderCircRadiusM() const { return _backgroundColliderCircRadiusM; }
		[[nodiscard]] box2d::ColliderType ForegroundColliderType() const { return _foregroundColliderType; }
		[[nodiscard]] VecF ForegroundColliderOriginToOriginVecM() const { return _foregroundColliderOriginToOriginVecM; }
		[[nodiscard]] VecF ForegroundColliderRectDimsM() const { return _foregroundColliderRectDimsM; }
		[[nodiscard]] float ForegroundColliderCircRadiusM() const { return _foregroundColliderCircRadiusM; }
		[[nodiscard]] bool IsBackgroundTile() const { return _isBackgroundTile; }
		[[nodiscard]] const std::vector<m2g::pb::ItemType>& NamedItems() const { return _namedItems; }

		[[nodiscard]] SDL_Texture* Texture(SpriteVariant spriteVariant = {}) const;
		[[nodiscard]] VecF TextureTotalDims(SpriteVariant spriteVariant) const;
		[[nodiscard]] const RectI& Rect(SpriteVariant spriteVariant) const;

		/// Ratio of screen pixels to sprite pixels
		/// Multiply sprite dimensions (srcpx) with this number to convert them to screen dimensions (dstpx).
		[[nodiscard]] float SheetToScreenPixelMultiplier() const;

		/// Returns a vector from the sprite's center pixel to the sprite's origin.
		[[nodiscard]] VecF CenterToOriginSrcpx(SpriteVariant spriteVariant) const;

		/// Returns a vector from the sprite's center pixel to the sprite's graphical origin in screen dimensions
		/// (dstpx).
		[[nodiscard]] VecF CenterToOriginDstpx(const SpriteVariant spriteVariant) const {
			// Convert from source pixels to destination pixels
			return CenterToOriginSrcpx(spriteVariant) * SheetToScreenPixelMultiplier();
		}
	};

	// Filters

	inline bool IsSpriteBackgroundTile(const Sprite& sprite) { return sprite.IsBackgroundTile(); }

	// Transformers

	inline m2g::pb::SpriteType ToSpriteType(const Sprite& sprite) { return sprite.Type(); }

	// Helpers

	std::vector<Sprite> LoadSprites(const std::vector<SpriteSheet>& spriteSheets,
			const google::protobuf::RepeatedPtrField<pb::TextLabel>& textLabels,
			SpriteEffectsSheet& spriteEffectsSheet, SDL_Renderer* renderer, TTF_Font* font, int fontSize, bool lightning);
	std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> ListLevelEditorObjectSprites(const std::filesystem::path& objects_path);
	void_expected MoveBackground(int from, int to, const std::string& level);
}
