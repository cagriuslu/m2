#include <rpg/Graphic.h>
#include <m2/Game.h>

void DrawAddons(const m2::Graphic& gfx, float health) {
		const auto& sprite = *std::get<const m2::Sprite*>(gfx.visual);

		SDL_Rect dst_rect{};
		if (m2::is_projection_type_parallel(M2_LEVEL.ProjectionType())) {
			const auto src_rect = static_cast<SDL_Rect>(sprite.Rect());
			const auto screen_origin_to_sprite_center_px_vec = sprite.ScreenOriginToCenterVecOutpx(gfx.owner().position, m2::SpriteVariant{});
			dst_rect = SDL_Rect{
				m2::iround(screen_origin_to_sprite_center_px_vec.x - (m2::F(src_rect.w) * M2_GAME.Dimensions().OutputPixelsPerMeter() / m2::F(sprite.Ppm()) / 2.0f)),
				m2::iround(screen_origin_to_sprite_center_px_vec.y + (m2::F(src_rect.h) * M2_GAME.Dimensions().OutputPixelsPerMeter() * 11.0f / m2::F(sprite.Ppm()) / 2.0f / 10.0f)), // Give an offset of 1.1
				m2::iround(M2_GAME.Dimensions().OutputPixelsPerMeter()),
				m2::iround(M2_GAME.Dimensions().OutputPixelsPerMeter() * 12.0f / 100.0f) // 0.15 m height
		};
		} else {
			const auto obj_position = gfx.owner().position;
			// Place add-on below the sprite
			const auto addon_position = m3::VecF{obj_position.x, obj_position.y, -0.2f};
			if (const auto projected_addon_position = screen_origin_to_projection_along_camera_plane_dstpx(addon_position)) {
				const auto rect = m2::RectI::centered_around(m2::VecI{*projected_addon_position}, m2::iround(M2_GAME.Dimensions().OutputPixelsPerMeter()), m2::iround(M2_GAME.Dimensions().OutputPixelsPerMeter() * 12.0f / 100.0f));
				dst_rect = static_cast<SDL_Rect>(rect);
			}
		}

		// Black background
		SDL_SetRenderDrawColor(M2_GAME.renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(M2_GAME.renderer, &dst_rect);

		// Green part
		const float percentage = health < 0.0f ? 0.0f : (1.0f < health) ? 1.0f : health;
		const auto green_rect = SDL_Rect{dst_rect.x + 1, dst_rect.y + 1, m2::I(roundf(percentage * m2::F(dst_rect.w - 2))), dst_rect.h - 2};
		SDL_SetRenderDrawColor(M2_GAME.renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(M2_GAME.renderer, &green_rect);
}
