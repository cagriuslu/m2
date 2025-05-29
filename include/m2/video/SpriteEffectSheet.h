#pragma once
#include <m2/video/DynamicSheet.h>
#include <m2/video/SpriteSheet.h>

namespace m2 {
	class SpriteEffectsSheet : DynamicSheet {
	public:
		explicit SpriteEffectsSheet(SDL_Renderer* renderer);
		using DynamicSheet::Texture;
		RectI create_mask_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::Color& mask_color, bool lightning);
		RectI create_foreground_companion_effect(const SpriteSheet& sheet, const pb::RectI& rect, const google::protobuf::RepeatedPtrField<pb::RectI>& rect_pieces, bool lightning);
		RectI create_grayscale_effect(const SpriteSheet& sheet, const pb::RectI& rect, bool lightning);
		RectI create_image_adjustment_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::ImageAdjustment& image_adjustment, bool lightning);
		RectI create_blurred_drop_shadow_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::BlurredDropShadow& blurred_drop_shadow, bool lightning);

		[[nodiscard]] int texture_width() const { return Width(); }
		[[nodiscard]] int texture_height() const { return Height(); }
	};

	void FillMaskEffect(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect, const RGBA& maskColor);
}
