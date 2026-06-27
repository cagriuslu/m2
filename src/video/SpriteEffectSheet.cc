#include <m2/video/SpriteEffectSheet.h>
#include <m2/common/math/Gaussian.h>
#include <m2/Log.h>
#include <numeric>

m2::RectI m2::SpriteEffectsSheet::create_mask_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::Color& mask_color) {
	return *AllocateAndMutate(rect.w(), rect.h(), [&](thirdparty::video::Surface& surface, const RectI& area) {
		FillMaskEffect(sheet.surface(), RectI{rect}, surface, area, RGBA{mask_color});
	});
}
m2::RectI m2::SpriteEffectsSheet::create_foreground_companion_effect(const SpriteSheet& sheet, const pb::RectI& rect,
	const google::protobuf::RepeatedPtrField<pb::RectI>& rect_pieces) {
	return *AllocateAndMutate(rect.w(), rect.h(), [&](thirdparty::video::Surface& dstSurface, const RectI& area) {
		FillForegroundCompanion(sheet.surface(), RectI{rect}, dstSurface, area, rect_pieces);
	}, false);
}
m2::RectI m2::SpriteEffectsSheet::create_grayscale_effect(const SpriteSheet& sheet, const pb::RectI& rect) {
	return *AllocateAndMutate(rect.w(), rect.h(), [&](thirdparty::video::Surface& dstSurface, const RectI& area) {
		FillGrayscaleEffect(sheet.surface(), RectI{rect}, dstSurface, area);
	});
}
m2::RectI m2::SpriteEffectsSheet::create_image_adjustment_effect(const SpriteSheet& sheet, const pb::RectI& rect,
	const pb::ImageAdjustment& imageAdjustment) {
	return *AllocateAndMutate(rect.w(), rect.h(), [&](thirdparty::video::Surface& dstSurface, const RectI& area) {
		FillImageAdjustmentEffect(sheet.surface(), RectI{rect}, dstSurface, area, imageAdjustment);
	});
}
m2::RectI m2::SpriteEffectsSheet::create_blurred_drop_shadow_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::BlurredDropShadow& blurredDropShadow) {
	return *AllocateAndMutate(rect.w(), rect.h(), [&](thirdparty::video::Surface& dstSurface, const RectI& area) {
		FillBlurredDropShadowEffect(sheet.surface(), RectI{rect}, dstSurface, area, blurredDropShadow);
	});
}

void m2::FillMaskEffect(const thirdparty::video::Surface& srcSurface, const RectI& srcRect, thirdparty::video::Surface& dstSurface, const RectI& dstRect, const RGBA& maskColor) {
	// Check pixel strides
	if (srcSurface.BytesPerPixel() != 4 || dstSurface.BytesPerPixel() != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	srcSurface.Lock();
	dstSurface.Lock();
	for (int y = srcRect.y; y < srcRect.y + srcRect.h; ++y) {
		for (int x = srcRect.x; x < srcRect.x + srcRect.w; ++x) {
			// Read src pixel
			const auto src = srcSurface.GetPixel(x, y);

			// Color dst pixel: opaque src pixels get the mask color, transparent ones become fully transparent
			dstSurface.SetPixel(x - srcRect.x + dstRect.x, y - srcRect.y + dstRect.y, src.a ? maskColor : RGBA{0, 0, 0, 0});
		}
	}
	dstSurface.Unlock();
	srcSurface.Unlock();
}
void m2::FillForegroundCompanion(const thirdparty::video::Surface& srcSurface, const RectI& srcRect, thirdparty::video::Surface& dstSurface, const RectI& dstRect, const google::protobuf::RepeatedPtrField<pb::RectI>& rectPieces) {
	for (const auto& rectPiece : rectPieces) {
		const auto sourceRect = RectI{rectPiece};
		const auto destinationRect = RectI{dstRect.x + rectPiece.x() - srcRect.x, dstRect.y + rectPiece.y() - srcRect.y, rectPiece.w(), rectPiece.h()};
		m2SucceedOrThrowError(dstSurface.Blit(srcSurface, sourceRect, destinationRect));
	}
}
void m2::FillGrayscaleEffect(const thirdparty::video::Surface& srcSurface, const RectI& srcRect, thirdparty::video::Surface& dstSurface, const RectI& dstRect) {
	// Check pixel stride
	if (srcSurface.BytesPerPixel() != 4 || dstSurface.BytesPerPixel() != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	srcSurface.Lock();
	dstSurface.Lock();
	for (int y = srcRect.y; y < srcRect.y + srcRect.h; ++y) {
		for (int x = srcRect.x; x < srcRect.x + srcRect.w; ++x) {
			// Read src pixel
			const auto src = srcSurface.GetPixel(x, y);
			// Apply weights
			const float rf = 0.299f * static_cast<float>(src.r) / 255.0f;
			const float gf = 0.587f * static_cast<float>(src.g) / 255.0f;
			const float bf = 0.114f * static_cast<float>(src.b) / 255.0f;
			// Convert back to int
			const auto bw = static_cast<uint8_t>(roundf((rf + gf + bf) * 255.0f));
			// Color dst pixel
			dstSurface.SetPixel(x - srcRect.x + dstRect.x, y - srcRect.y + dstRect.y, RGBA{bw, bw, bw, src.a});
		}
	}
	dstSurface.Unlock();
	srcSurface.Unlock();
}
void m2::FillImageAdjustmentEffect(const thirdparty::video::Surface& srcSurface, const RectI& srcRect, thirdparty::video::Surface& dstSurface, const RectI& dstRect, const pb::ImageAdjustment& imageAdjustment) {
	// Check pixel stride
	if (srcSurface.BytesPerPixel() != 4 || dstSurface.BytesPerPixel() != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	srcSurface.Lock();
	dstSurface.Lock();
	for (int y = srcRect.y; y < srcRect.y + srcRect.h; ++y) {
		for (int x = srcRect.x; x < srcRect.x + srcRect.w; ++x) {
			// Read src pixel
			const auto src = srcSurface.GetPixel(x, y);
			// Apply weights
			const float rf = imageAdjustment.brightness_multiplier() * static_cast<float>(src.r) / 255.0f;
			const float gf = imageAdjustment.brightness_multiplier() * static_cast<float>(src.g) / 255.0f;
			const float bf = imageAdjustment.brightness_multiplier() * static_cast<float>(src.b) / 255.0f;
			// Convert back to int
			const auto rn = static_cast<uint8_t>(roundf(rf * 255.0f));
			const auto gn = static_cast<uint8_t>(roundf(gf * 255.0f));
			const auto bn = static_cast<uint8_t>(roundf(bf * 255.0f));
			// Color dst pixel
			dstSurface.SetPixel(x - srcRect.x + dstRect.x, y - srcRect.y + dstRect.y, RGBA{rn, gn, bn, src.a});
		}
	}
	dstSurface.Unlock();
	srcSurface.Unlock();
}
void m2::FillBlurredDropShadowEffect(const thirdparty::video::Surface& srcSurface, const RectI& srcRect, thirdparty::video::Surface& dstSurface, const RectI& dstRect, const pb::BlurredDropShadow& blurredDropShadow) {
	// Check pixel stride
	if (srcSurface.BytesPerPixel() != 4 || dstSurface.BytesPerPixel() != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	// Create the image kernel
	auto kernel = CreateGaussianKernel(blurredDropShadow.blur_radius(), blurredDropShadow.standard_deviation());
	// Size of the one side of the matrix
	const auto side_size = blurredDropShadow.blur_radius() * 2 + 1;

	// Returns the normalized alpha channel value of the pixel at the given coordinate. If the coordinates lay outside
	// the rect, 0 is returned. Assumes that the surface is already locked.
	auto pixel_alpha_reader = [srcRect](const thirdparty::video::Surface& surface, const int x, const int y) {
		if (x < srcRect.x || srcRect.x + srcRect.w <= x
			|| y < srcRect.y || srcRect.y + srcRect.h <= y) {
			return 0.0f;
		}
		// Return the normalized alpha component
		return static_cast<float>(surface.GetPixel(x, y).a) / 255.0f;
	};

	srcSurface.Lock();
	dstSurface.Lock();
	for (int y = srcRect.y; y < srcRect.y + srcRect.h; ++y) {
		for (int x = srcRect.x; x < srcRect.x + srcRect.w; ++x) {
			// Apply the kernel only to the alpha channel, other channels are full black
			// Prepare the input by fetching the pixels from the surface
			std::vector input(kernel.size(), 0.0f);
			for (int input_y = 0; input_y < side_size; ++input_y) {
				for (int input_x = 0; input_x < side_size; ++input_x) {
					input[input_y * side_size + input_x] = pixel_alpha_reader(srcSurface, x + input_x - blurredDropShadow.blur_radius(), y + input_y - blurredDropShadow.blur_radius());
				}
			}
			// Multiply the corresponding elements with the kernel
			std::vector output(kernel.size(), 0.0f);
			std::transform(input.begin(), input.end(), kernel.begin(), std::back_inserter(output), std::multiplies<float>{});
			// Sum the elements
			float sum = std::accumulate(output.begin(), output.end(), 0.0f);
			// Apply final transparency
			sum *= blurredDropShadow.final_transparency();
			// Clamp to [0,1]
			sum = std::clamp(sum, 0.0f, 1.0f);
			// Convert back to int
			const auto an = RoundI(sum * 255.0f);
			// Color dst pixel
			dstSurface.SetPixel(x - srcRect.x + dstRect.x, y - srcRect.y + dstRect.y, RGBA{0, 0, 0, an});
		}
	}
	dstSurface.Unlock();
	srcSurface.Unlock();
}
