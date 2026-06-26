#include <rpg/Graphic.h>
#include <m2/thirdparty/video/Shapes.h>
#include <m2/Game.h>

void DrawAddons(const m2::Graphic& gfx, float health) {
		const auto& sprite = *std::get<const m2::Sprite*>(gfx.visual);

		m2::RectI dst_rect{};
		if (m2::IsProjectionTypeParallel(M2_LEVEL.GetProjectionType())) {
			const auto src_rect = sprite.GetRect();
			const auto screen_origin_to_sprite_center_px_vec = sprite.ScreenOriginToCenterVecOutpx(gfx.position);
			dst_rect = m2::RectI{
				m2::RoundI(screen_origin_to_sprite_center_px_vec.GetX() - (m2::ToFloat(src_rect.w) * M2_GAME.Dimensions().OutputPixelsPerMeter() / m2::ToFloat(sprite.Ppm()) / 2.0f)),
				m2::RoundI(screen_origin_to_sprite_center_px_vec.GetY() + (m2::ToFloat(src_rect.h) * M2_GAME.Dimensions().OutputPixelsPerMeter() * 11.0f / m2::ToFloat(sprite.Ppm()) / 2.0f / 10.0f)), // Give an offset of 1.1
				m2::RoundI(M2_GAME.Dimensions().OutputPixelsPerMeter()),
				m2::RoundI(M2_GAME.Dimensions().OutputPixelsPerMeter() * 12.0f / 100.0f) // 0.15 m height
		};
		} else {
			const auto obj_position = gfx.position;
			// Place add-on below the sprite
			const auto addon_position = m3::VecF{obj_position.GetX(), obj_position.GetY(), -0.2f};
			if (const auto projected_addon_position = ScreenOriginToProjectionAlongCameraPlaneDstpx(addon_position)) {
				dst_rect = m2::RectI::CreateCenteredAround(m2::VecI{*projected_addon_position}, m2::RoundI(M2_GAME.Dimensions().OutputPixelsPerMeter()), m2::RoundI(M2_GAME.Dimensions().OutputPixelsPerMeter() * 12.0f / 100.0f));
			}
		}

		// Black background
		m2::thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), dst_rect, m2::RGBA{0, 0, 0, 255});

		// Green part
		const float percentage = health < 0.0f ? 0.0f : (1.0f < health) ? 1.0f : health;
		const auto green_rect = m2::RectI{dst_rect.x + 1, dst_rect.y + 1, m2::I(roundf(percentage * m2::ToFloat(dst_rect.w - 2))), dst_rect.h - 2};
		m2::thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), green_rect, m2::RGBA{0, 255, 0, 255});
}
