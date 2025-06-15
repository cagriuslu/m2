#pragma once
#include <m2/video/DynamicSheet.h>
#include <m2/video/SpriteSheet.h>

namespace m2 {
	class SpriteEffectsSheet : DynamicSheet {
	public:
		using DynamicSheet::DynamicSheet;
		using DynamicSheet::Texture;

		RectI create_mask_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::Color& mask_color);
		RectI create_foreground_companion_effect(const SpriteSheet& sheet, const pb::RectI& rect, const google::protobuf::RepeatedPtrField<pb::RectI>& rect_pieces);
		RectI create_grayscale_effect(const SpriteSheet& sheet, const pb::RectI& rect);
		RectI create_image_adjustment_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::ImageAdjustment& imageAdjustment);
		RectI create_blurred_drop_shadow_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::BlurredDropShadow& blurredDropShadow);

		[[nodiscard]] int texture_width() const { return Width(); }
		[[nodiscard]] int texture_height() const { return Height(); }
	};

	void FillMaskEffect(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect, const RGBA& maskColor);
	void FillForegroundCompanion(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect, const google::protobuf::RepeatedPtrField<pb::RectI>& rectPieces);
	void FillGrayscaleEffect(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect);
	void FillImageAdjustmentEffect(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect, const pb::ImageAdjustment& imageAdjustment);
	void FillBlurredDropShadowEffect(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect, const pb::BlurredDropShadow& blurredDropShadow);
}
