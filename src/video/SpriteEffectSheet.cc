#include <m2/video/SpriteEffectSheet.h>
#include <m2/detail/Gaussian.h>
#include <numeric>

m2::SpriteEffectsSheet::SpriteEffectsSheet(SDL_Renderer* renderer) : DynamicSheet(renderer) {}
m2::RectI m2::SpriteEffectsSheet::create_mask_effect(
    const SpriteSheet& sheet, const pb::RectI& rect, const pb::Color& mask_color, bool lightning) {
	auto [dst_surface, dst_rect] = Alloc(rect.w(), rect.h());

	// Check pixel stride
	auto* src_surface = sheet.surface();
	if (src_surface->format->BytesPerPixel != 4 || dst_surface->format->BytesPerPixel != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	// Prepare mask color
	uint32_t dst_color =
	    ((mask_color.r() >> dst_surface->format->Rloss) << dst_surface->format->Rshift) & dst_surface->format->Rmask;
	dst_color |=
	    ((mask_color.g() >> dst_surface->format->Gloss) << dst_surface->format->Gshift) & dst_surface->format->Gmask;
	dst_color |=
	    ((mask_color.b() >> dst_surface->format->Bloss) << dst_surface->format->Bshift) & dst_surface->format->Bmask;
	dst_color |=
	    ((mask_color.a() >> dst_surface->format->Aloss) << dst_surface->format->Ashift) & dst_surface->format->Amask;

	SDL_LockSurface(src_surface);
	SDL_LockSurface(dst_surface);

	for (int y = rect.y(); y < rect.y() + rect.h(); ++y) {
		for (int x = rect.x(); x < rect.x() + rect.w(); ++x) {
			// Read src pixel
			auto* src_pixels = static_cast<uint32_t*>(src_surface->pixels);
			auto src_pixel = *(src_pixels + (x + y * src_surface->w));

			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dst_surface->pixels);
			auto* dst_pixel = dst_pixels + ((x - rect.x()) + (y - rect.y() + dst_rect.y) * dst_surface->w);
			*dst_pixel = (src_pixel & src_surface->format->Amask) ? dst_color : 0;
		}
	}

	SDL_UnlockSurface(dst_surface);
	SDL_UnlockSurface(src_surface);

	RecreateTexture(lightning);

	return dst_rect;
}
m2::RectI m2::SpriteEffectsSheet::create_foreground_companion_effect(
    const SpriteSheet& sheet, const pb::RectI& rect, const google::protobuf::RepeatedPtrField<pb::RectI>& rect_pieces,
    bool lightning) {
	auto [dst_surface, dst_rect] = Alloc(rect.w(), rect.h());

	for (const auto& rect_piece : rect_pieces) {
		auto blit_src_rect = static_cast<SDL_Rect>(RectI{rect_piece});
		auto blit_dst_rect = SDL_Rect{
		    dst_rect.x + rect_piece.x() - rect.x(), dst_rect.y + rect_piece.y() - rect.y(), rect_piece.w(),
		    rect_piece.h()};
		SDL_BlitSurface(sheet.surface(), &blit_src_rect, dst_surface, &blit_dst_rect);
	}

	RecreateTexture(lightning);

	return dst_rect;
}
m2::RectI m2::SpriteEffectsSheet::create_grayscale_effect(
    const SpriteSheet& sheet, const pb::RectI& rect, bool lightning) {
	auto [dst_surface, dst_rect] = Alloc(rect.w(), rect.h());

	// Check pixel stride
	auto* src_surface = sheet.surface();
	if (src_surface->format->BytesPerPixel != 4 || dst_surface->format->BytesPerPixel != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	SDL_LockSurface(src_surface);
	SDL_LockSurface(dst_surface);

	for (int y = rect.y(); y < rect.y() + rect.h(); ++y) {
		for (int x = rect.x(); x < rect.x() + rect.w(); ++x) {
			// Read src pixel
			auto* src_pixels = static_cast<uint32_t*>(src_surface->pixels);
			auto src_pixel = *(src_pixels + (x + y * src_surface->w));
			// Decompose to RPG
			uint8_t r, g, b, a;
			SDL_GetRGBA(src_pixel, src_surface->format, &r, &g, &b, &a);
			// Apply weights
			float rf = 0.299f * static_cast<float>(r) / 255.0f;
			float gf = 0.587f * static_cast<float>(g) / 255.0f;
			float bf = 0.114f * static_cast<float>(b) / 255.0f;
			// Convert back to int
			auto bw = (uint8_t)roundf((rf + gf + bf) * 255.0f);
			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dst_surface->pixels);
			auto* dst_pixel = dst_pixels + ((x - rect.x()) + (y - rect.y() + dst_rect.y) * dst_surface->w);
			*dst_pixel = SDL_MapRGBA(dst_surface->format, bw, bw, bw, a);
		}
	}

	SDL_UnlockSurface(dst_surface);
	SDL_UnlockSurface(src_surface);

	RecreateTexture(lightning);

	return dst_rect;
}
m2::RectI m2::SpriteEffectsSheet::create_image_adjustment_effect(
    const SpriteSheet& sheet, const pb::RectI& rect, const pb::ImageAdjustment& image_adjustment, bool lightning) {
	auto [dst_surface, dst_rect] = Alloc(rect.w(), rect.h());

	// Check pixel stride
	auto* src_surface = sheet.surface();
	if (src_surface->format->BytesPerPixel != 4 || dst_surface->format->BytesPerPixel != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	SDL_LockSurface(src_surface);
	SDL_LockSurface(dst_surface);

	for (int y = rect.y(); y < rect.y() + rect.h(); ++y) {
		for (int x = rect.x(); x < rect.x() + rect.w(); ++x) {
			// Read src pixel
			auto* src_pixels = static_cast<uint32_t*>(src_surface->pixels);
			auto src_pixel = *(src_pixels + (x + y * src_surface->w));
			// Decompose to RPG
			uint8_t r, g, b, a;
			SDL_GetRGBA(src_pixel, src_surface->format, &r, &g, &b, &a);
			// Apply weights
			float rf = image_adjustment.brightness_multiplier() * static_cast<float>(r) / 255.0f;
			float gf = image_adjustment.brightness_multiplier() * static_cast<float>(g) / 255.0f;
			float bf = image_adjustment.brightness_multiplier() * static_cast<float>(b) / 255.0f;
			// Convert back to int
			auto rn = (uint8_t)roundf(rf * 255.0f);
			auto gn = (uint8_t)roundf(gf * 255.0f);
			auto bn = (uint8_t)roundf(bf * 255.0f);
			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dst_surface->pixels);
			auto* dst_pixel = dst_pixels + ((x - rect.x()) + (y - rect.y() + dst_rect.y) * dst_surface->w);
			*dst_pixel = SDL_MapRGBA(dst_surface->format, rn, gn, bn, a);
		}
	}

	SDL_UnlockSurface(dst_surface);
	SDL_UnlockSurface(src_surface);

	RecreateTexture(lightning);

	return dst_rect;
}
m2::RectI m2::SpriteEffectsSheet::create_blurred_drop_shadow_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::BlurredDropShadow& blurred_drop_shadow, bool lightning) {
	auto [dst_surface, dst_rect] = Alloc(rect.w(), rect.h());

	// Check pixel stride
	auto* src_surface = sheet.surface();
	if (src_surface->format->BytesPerPixel != 4 || dst_surface->format->BytesPerPixel != 4) {
		throw M2_ERROR("Surface has unsupported pixel format");
	}

	// Create the image kernel
	auto kernel = create_gaussian_kernel(blurred_drop_shadow.blur_radius(), blurred_drop_shadow.standard_deviation());
	// Size of the one side of the matrix
	auto side_size = blurred_drop_shadow.blur_radius() * 2 + 1;

	// Returns the normalized alpha channel value of the pixel at the given coordinate. If the coordinates lay outside
	// the rect, 0 is returned. Assumes that the surface is already locked.
	auto pixel_alpha_reader = [rect](SDL_Surface* surface, int x, int y) {
		if (x < rect.x() || rect.x() + rect.w() <= x
			|| y < rect.y() || rect.y() + rect.h() <= y) {
			return 0.0f;
		}
		// Read src pixel
		auto* src_pixels = static_cast<uint32_t*>(surface->pixels);
		auto src_pixel = *(src_pixels + (x + y * surface->w));
		// Decompose to RPG
		uint8_t r, g, b, a;
		SDL_GetRGBA(src_pixel, surface->format, &r, &g, &b, &a);
		// Return the normalized alpha component
		return static_cast<float>(a) / 255.0f;
	};

	SDL_LockSurface(src_surface);
	SDL_LockSurface(dst_surface);

	for (int y = rect.y(); y < rect.y() + rect.h(); ++y) {
		for (int x = rect.x(); x < rect.x() + rect.w(); ++x) {
			// Apply the kernel only to the alpha channel, other channels are full black
			// Prepare the input by fetching the pixels from the surface
			std::vector<float> input(kernel.size(), 0.0f);
			for (int input_y = 0; input_y < side_size; ++input_y) {
				for (int input_x = 0; input_x < side_size; ++input_x) {
					input[input_y * side_size + input_x] = pixel_alpha_reader(src_surface, x + input_x - blurred_drop_shadow.blur_radius(), y + input_y - blurred_drop_shadow.blur_radius());
				}
			}
			// Multiply the corresponding elements with the kernel
			std::vector<float> output(kernel.size(), 0.0f);
			std::transform(input.begin(), input.end(), kernel.begin(), std::back_inserter(output), std::multiplies<float>{});
			// Sum the elements
			float sum = std::accumulate(output.begin(), output.end(), 0.0f);
			// Apply final transparency
			sum *= blurred_drop_shadow.final_transparency();
			// Clamp to [0,1]
			sum = std::clamp(sum, 0.0f, 1.0f);
			// Convert back to int
			auto an = iround(sum * 255.0f);
			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dst_surface->pixels);
			auto* dst_pixel = dst_pixels + ((x - rect.x()) + (y - rect.y() + dst_rect.y) * dst_surface->w);
			*dst_pixel = SDL_MapRGBA(dst_surface->format, 0, 0, 0, an);
		}
	}

	SDL_UnlockSurface(dst_surface);
	SDL_UnlockSurface(src_surface);

	RecreateTexture(lightning);

	return dst_rect;
}
