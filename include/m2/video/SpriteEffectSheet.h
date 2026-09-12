#pragma once
#include <m2/video/DynamicSheet.h>
#include <m2/video/SpriteSheet.h>

namespace m2 {
	class SpriteEffectsSheet : DynamicSheet {
	public:
		using DynamicSheet::DynamicSheet;
		using DynamicSheet::Texture;

		/// Allocates effects on the sheet. Handed to the CreateEffects callback; every allocation made through it shares
		/// one GPU texture rebuild.
		class Batch {
			friend SpriteEffectsSheet;
			MutableInterface& _mutableInterface;
			explicit Batch(MutableInterface&);
		public:
			Batch(const Batch&) = delete;
			Batch& operator=(const Batch&) = delete;
			Batch(Batch&&) = delete;
			Batch& operator=(Batch&&) = delete;

			RectI CreateMaskEffect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::Color& maskColor);
			RectI CreateForegroundCompanionEffect(const SpriteSheet& sheet, const pb::RectI& rect, const google::protobuf::RepeatedPtrField<pb::RectI>& rectPieces);
			RectI CreateGrayscaleEffect(const SpriteSheet& sheet, const pb::RectI& rect);
			RectI CreateImageAdjustmentEffect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::ImageAdjustment& imageAdjustment);
			RectI CreateBlurredDropShadowEffect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::BlurredDropShadow& blurredDropShadow);
		};

		/// Runs the callback with a Batch, then rebuilds the GPU texture once. Every reference previously obtained from
		/// Texture() is invalidated by this call.
		void CreateEffects(const std::function<void(Batch&)>& cb);

		[[nodiscard]] int texture_width() const { return Width(); }
		[[nodiscard]] int texture_height() const { return Height(); }
	};

	/// The destination surface must be locked by the caller. The source surface is locked internally.
	void FillMaskEffect(const thirdparty::video::Surface& srcSurface, const RectI& srcRect, thirdparty::video::Surface& dstSurface, const RectI& dstRect, const RGBA& maskColor);
	/// Neither surface may be locked: this blits, and SDL refuses to blit into or out of a locked surface.
	void FillForegroundCompanion(const thirdparty::video::Surface& srcSurface, const RectI& srcRect, thirdparty::video::Surface& dstSurface, const RectI& dstRect, const google::protobuf::RepeatedPtrField<pb::RectI>& rectPieces);
	/// The destination surface must be locked by the caller. The source surface is locked internally.
	void FillGrayscaleEffect(const thirdparty::video::Surface& srcSurface, const RectI& srcRect, thirdparty::video::Surface& dstSurface, const RectI& dstRect);
	/// The destination surface must be locked by the caller. The source surface is locked internally.
	void FillImageAdjustmentEffect(const thirdparty::video::Surface& srcSurface, const RectI& srcRect, thirdparty::video::Surface& dstSurface, const RectI& dstRect, const pb::ImageAdjustment& imageAdjustment);
	/// The destination surface must be locked by the caller. The source surface is locked internally.
	void FillBlurredDropShadowEffect(const thirdparty::video::Surface& srcSurface, const RectI& srcRect, thirdparty::video::Surface& dstSurface, const RectI& dstRect, const pb::BlurredDropShadow& blurredDropShadow);
}
