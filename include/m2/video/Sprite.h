#pragma once
#include <m2/video/SpriteEffectSheet.h>
#include <m2/video/SpriteSheet.h>
#include <m2/sdl/TextTexture.h>
#include <m2/math/VecF.h>
#include <m2/protobuf/Detail.h>
#include <m2g_ObjectType.pb.h>

namespace m2 {
	class Sprite final {
		const SpriteSheet* _spriteSheet{};
		const SpriteEffectsSheet* _effectsSheet{};
		const pb::Sprite* _pb{};
		const pb::Sprite* _originalPb{};

		std::optional<RectI> _foregroundCompanionSpriteEffectsSheetRect;
		std::optional<VecF> _foregroundCompanionCenterToOriginVecPx;
		std::optional<VecF> _foregroundCompanionCenterToOriginVecM;
		RectI _rect;
		float _originalRotationRad{};
		int _ppm{};
		VecF _centerToOriginVecPx;
		VecF _centerToOriginVecM;
		std::vector<m2g::pb::ItemType> _namedItems;

	   public:
		Sprite() = default;
		Sprite(const std::vector<SpriteSheet>& spriteSheets, const SpriteSheet& spriteSheet,
				SpriteEffectsSheet& spriteEffectsSheet, const pb::Sprite& sprite);

		// Accessors

		[[nodiscard]] const SpriteSheet& Sheet() const { return *_spriteSheet; }
		[[nodiscard]] const pb::Sprite& Pb() const { return *_pb; }
		[[nodiscard]] const pb::Sprite& OriginalPb() const { return *_originalPb; }
		[[nodiscard]] m2g::pb::SpriteType Type() const { return _pb->type(); }
		[[nodiscard]] m2g::pb::SpriteType OriginalType() const { return _originalPb->type(); }
		[[nodiscard]] bool HasForegroundCompanion() const { return _foregroundCompanionCenterToOriginVecM.has_value(); }
		[[nodiscard]] VecF ForegroundCompanionCenterToOriginVecPx() const { return _foregroundCompanionCenterToOriginVecPx.value(); }
		[[nodiscard]] VecF ForegroundCompanionCenterToOriginVecM() const { return _foregroundCompanionCenterToOriginVecM.value(); }
		[[nodiscard]] float OriginalRotationRadians() const { return _originalRotationRad; }
		[[nodiscard]] int Ppm() const { return _ppm; }
		[[nodiscard]] const VecF& CenterToOriginVecPx() const { return _centerToOriginVecPx; }
		[[nodiscard]] const VecF& CenterToOriginVecM() const { return _centerToOriginVecM; }
		[[nodiscard]] bool IsBackgroundTile() const { return _originalPb->regular().is_background_tile(); }
		[[nodiscard]] const std::vector<m2g::pb::ItemType>& NamedItems() const { return _namedItems; }

		[[nodiscard]] SDL_Texture* GetTexture(bool foregroundCompanion = false) const;
		[[nodiscard]] VecF TextureTotalDims(bool foregroundCompanion = false) const;
		[[nodiscard]] const RectI& GetRect(bool foregroundCompanion = false) const;

		/// Ratio of screen pixels to sprite pixels
		/// Multiply sprite dimensions (srcpx) with this number to convert them to screen dimensions (dstpx).
		[[nodiscard]] float SourceToOutputPixelMultiplier() const;

		/// Returns a vector from the sprite's center pixel to the sprite's origin.
		[[nodiscard]] VecF CenterToOriginVecSrcpx(bool foregroundCompanion = false) const;

		/// Returns a vector from the sprite's center to the sprite's origin in output pixel units.
		[[nodiscard]] VecF CenterToOriginVecOutpx(const bool foregroundCompanion = false) const {
			// Convert from source pixels to destination pixels
			return CenterToOriginVecSrcpx(foregroundCompanion) * SourceToOutputPixelMultiplier();
		}

		/// Returns a vector from the screen origin (top-left) to the center of this sprite in output pixel units.
		[[nodiscard]] VecF ScreenOriginToCenterVecOutpx(const VecF& position, bool foregroundCompanion = false) const;

		/// Draws the given variant of the sprite with the given angle at world position in 2D mode. is_foreground and
		/// z parameters are ignored, and only provided for API compatibility with DrawIn3dWorld.
		void DrawIn2dWorld(const VecF& position, bool foregroundCompanion, float angle, bool is_foreground = {}, float z = {}) const;
		/// Draws the given variant of the sprite with the given angle at world position in 3D mode. If is_foreground
		/// is true, the sprite is drawn standing up, with its origin at z.
		void DrawIn3dWorld(const VecF& position, bool foregroundCompanion, float angle, bool is_foreground, float z) const;
	};

	// Filters

	bool IsSpriteBackgroundTile(const std::variant<Sprite,pb::TextLabel>&);

	// Transformers

	m2g::pb::SpriteType ToSpriteType(const std::variant<Sprite,pb::TextLabel>&);

	// Helpers

	std::vector<std::variant<Sprite, pb::TextLabel>> LoadSprites(const std::vector<SpriteSheet>& spriteSheets,
			const google::protobuf::RepeatedPtrField<pb::TextLabel>& textLabels,
			SpriteEffectsSheet& spriteEffectsSheet);
	void_expected MoveBackground(int from, int to, const std::string& level);
}
