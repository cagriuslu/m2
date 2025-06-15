#include <m2/video/SpriteEffectSheet.h>
#include <m2/detail/Gaussian.h>
#include <m2/Log.h>
#include <numeric>

m2::RectI m2::SpriteEffectsSheet::create_mask_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::Color& mask_color) {
	return *AllocateAndMutate(rect.w(), rect.h(), [&](SDL_Surface* surface, const RectI& area) {
		FillMaskEffect(sheet.surface(), RectI{rect}, surface, area, RGBA{mask_color});
	});
}
m2::RectI m2::SpriteEffectsSheet::create_foreground_companion_effect(const SpriteSheet& sheet, const pb::RectI& rect,
	const google::protobuf::RepeatedPtrField<pb::RectI>& rect_pieces) {
	return *AllocateAndMutate(rect.w(), rect.h(), [&](SDL_Surface* dstSurface, const RectI& area) {
		FillForegroundCompanion(sheet.surface(), RectI{rect}, dstSurface, area, rect_pieces);
	}, false);
}
m2::RectI m2::SpriteEffectsSheet::create_grayscale_effect(const SpriteSheet& sheet, const pb::RectI& rect) {
	return *AllocateAndMutate(rect.w(), rect.h(), [&](SDL_Surface* dstSurface, const RectI& area) {
		FillGrayscaleEffect(sheet.surface(), RectI{rect}, dstSurface, area);
	});
}
m2::RectI m2::SpriteEffectsSheet::create_image_adjustment_effect(const SpriteSheet& sheet, const pb::RectI& rect,
	const pb::ImageAdjustment& imageAdjustment) {
	return *AllocateAndMutate(rect.w(), rect.h(), [&](SDL_Surface* dstSurface, const RectI& area) {
		FillImageAdjustmentEffect(sheet.surface(), RectI{rect}, dstSurface, area, imageAdjustment);
	});
}
m2::RectI m2::SpriteEffectsSheet::create_blurred_drop_shadow_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::BlurredDropShadow& blurredDropShadow) {
	return *AllocateAndMutate(rect.w(), rect.h(), [&](SDL_Surface* dstSurface, const RectI& area) {
		FillBlurredDropShadowEffect(sheet.surface(), RectI{rect}, dstSurface, area, blurredDropShadow);
	});
}

void m2::FillMaskEffect(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect, const RGBA& maskColor) {
	// Check pixel strides
	if (srcSurface->format->BytesPerPixel != 4 || dstSurface->format->BytesPerPixel != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	// Prepare mask color
	uint32_t dst_color = ((maskColor.r >> dstSurface->format->Rloss) << dstSurface->format->Rshift) & dstSurface->format->Rmask;
	dst_color |= ((maskColor.g >> dstSurface->format->Gloss) << dstSurface->format->Gshift) & dstSurface->format->Gmask;
	dst_color |= ((maskColor.b >> dstSurface->format->Bloss) << dstSurface->format->Bshift) & dstSurface->format->Bmask;
	dst_color |= ((maskColor.a >> dstSurface->format->Aloss) << dstSurface->format->Ashift) & dstSurface->format->Amask;

	SDL_LockSurface(srcSurface);
	SDL_LockSurface(dstSurface);
	for (int y = srcRect.y; y < srcRect.y + srcRect.h; ++y) {
		for (int x = srcRect.x; x < srcRect.x + srcRect.w; ++x) {
			// Read src pixel
			const auto* srcPixels = static_cast<uint32_t*>(srcSurface->pixels);
			const auto srcPixel = *(srcPixels + (x + y * srcSurface->w));

			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dstSurface->pixels);
			auto* dst_pixel = dst_pixels + ((x - srcRect.x + dstRect.x) + (y - srcRect.y + dstRect.y) * dstSurface->w);
			*dst_pixel = (srcPixel & srcSurface->format->Amask) ? dst_color : 0;
		}
	}
	SDL_UnlockSurface(dstSurface);
	SDL_UnlockSurface(srcSurface);
}
void m2::FillForegroundCompanion(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect, const google::protobuf::RepeatedPtrField<pb::RectI>& rectPieces) {
	for (const auto& rectPiece : rectPieces) {
		auto blitSrcRect = static_cast<SDL_Rect>(RectI{rectPiece});
		auto blitDstRect = SDL_Rect{dstRect.x + rectPiece.x() - srcRect.x, dstRect.y + rectPiece.y() - srcRect.y, rectPiece.w(), rectPiece.h()};
		SDL_BlitSurface(srcSurface, &blitSrcRect, dstSurface, &blitDstRect);
	}
}
void m2::FillGrayscaleEffect(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect) {
	// Check pixel stride
	if (srcSurface->format->BytesPerPixel != 4 || dstSurface->format->BytesPerPixel != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	SDL_LockSurface(srcSurface);
	SDL_LockSurface(dstSurface);
	for (int y = srcRect.y; y < srcRect.y + srcRect.h; ++y) {
		for (int x = srcRect.x; x < srcRect.x + srcRect.w; ++x) {
			// Read src pixel
			const auto* src_pixels = static_cast<uint32_t*>(srcSurface->pixels);
			const auto src_pixel = *(src_pixels + (x + y * srcSurface->w));
			// Decompose to RPG
			uint8_t r, g, b, a;
			SDL_GetRGBA(src_pixel, srcSurface->format, &r, &g, &b, &a);
			// Apply weights
			const float rf = 0.299f * static_cast<float>(r) / 255.0f;
			const float gf = 0.587f * static_cast<float>(g) / 255.0f;
			const float bf = 0.114f * static_cast<float>(b) / 255.0f;
			// Convert back to int
			const auto bw = static_cast<uint8_t>(roundf((rf + gf + bf) * 255.0f));
			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dstSurface->pixels);
			auto* dst_pixel = dst_pixels + ((x - srcRect.x + dstRect.x) + (y - srcRect.y + dstRect.y) * dstSurface->w);
			*dst_pixel = SDL_MapRGBA(dstSurface->format, bw, bw, bw, a);
		}
	}
	SDL_UnlockSurface(dstSurface);
	SDL_UnlockSurface(srcSurface);
}
void m2::FillImageAdjustmentEffect(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect, const pb::ImageAdjustment& imageAdjustment) {
	// Check pixel stride
	if (srcSurface->format->BytesPerPixel != 4 || dstSurface->format->BytesPerPixel != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	SDL_LockSurface(srcSurface);
	SDL_LockSurface(dstSurface);
	for (int y = srcRect.y; y < srcRect.y + srcRect.h; ++y) {
		for (int x = srcRect.x; x < srcRect.x + srcRect.w; ++x) {
			// Read src pixel
			const auto* src_pixels = static_cast<uint32_t*>(srcSurface->pixels);
			const auto src_pixel = *(src_pixels + (x + y * srcSurface->w));
			// Decompose to RPG
			uint8_t r, g, b, a;
			SDL_GetRGBA(src_pixel, srcSurface->format, &r, &g, &b, &a);
			// Apply weights
			const float rf = imageAdjustment.brightness_multiplier() * static_cast<float>(r) / 255.0f;
			const float gf = imageAdjustment.brightness_multiplier() * static_cast<float>(g) / 255.0f;
			const float bf = imageAdjustment.brightness_multiplier() * static_cast<float>(b) / 255.0f;
			// Convert back to int
			const auto rn = static_cast<uint8_t>(roundf(rf * 255.0f));
			const auto gn = static_cast<uint8_t>(roundf(gf * 255.0f));
			const auto bn = static_cast<uint8_t>(roundf(bf * 255.0f));
			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dstSurface->pixels);
			auto* dst_pixel = dst_pixels + ((x - srcRect.x + dstRect.x) + (y - srcRect.y + dstRect.y) * dstSurface->w);
			*dst_pixel = SDL_MapRGBA(dstSurface->format, rn, gn, bn, a);
		}
	}
	SDL_UnlockSurface(dstSurface);
	SDL_UnlockSurface(srcSurface);
}
void m2::FillBlurredDropShadowEffect(SDL_Surface* srcSurface, const RectI& srcRect, SDL_Surface* dstSurface, const RectI& dstRect, const pb::BlurredDropShadow& blurredDropShadow) {
	// Check pixel stride
	if (srcSurface->format->BytesPerPixel != 4 || dstSurface->format->BytesPerPixel != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	// Create the image kernel
	auto kernel = CreateGaussianKernel(blurredDropShadow.blur_radius(), blurredDropShadow.standard_deviation());
	// Size of the one side of the matrix
	const auto side_size = blurredDropShadow.blur_radius() * 2 + 1;

	// Returns the normalized alpha channel value of the pixel at the given coordinate. If the coordinates lay outside
	// the rect, 0 is returned. Assumes that the surface is already locked.
	auto pixel_alpha_reader = [srcRect](SDL_Surface* surface, const int x, const int y) {
		if (x < srcRect.x || srcRect.x + srcRect.w <= x
			|| y < srcRect.y || srcRect.y + srcRect.h <= y) {
			return 0.0f;
		}
		// Read src pixel
		const auto* src_pixels = static_cast<uint32_t*>(surface->pixels);
		const auto src_pixel = *(src_pixels + (x + y * surface->w));
		// Decompose to RPG
		uint8_t r, g, b, a;
		SDL_GetRGBA(src_pixel, surface->format, &r, &g, &b, &a);
		// Return the normalized alpha component
		return static_cast<float>(a) / 255.0f;
	};

	SDL_LockSurface(srcSurface);
	SDL_LockSurface(dstSurface);
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
			auto* dst_pixels = static_cast<uint32_t*>(dstSurface->pixels);
			auto* dst_pixel = dst_pixels + ((x - srcRect.x + dstRect.x) + (y - srcRect.y + dstRect.y) * dstSurface->w);
			*dst_pixel = SDL_MapRGBA(dstSurface->format, 0, 0, 0, static_cast<uint8_t>(an));
		}
	}
	SDL_UnlockSurface(dstSurface);
	SDL_UnlockSurface(srcSurface);
}
