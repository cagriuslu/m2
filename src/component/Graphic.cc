#include <m2/Component.h>
#include <m2/Game.hh>
#include "m2/component/Graphic.h"
#include <m2/Object.h>

m2::Vec2f m2::camera_to_position_m(const Vec2f& position) {
	auto* camera = GAME.objects.get(GAME.cameraId);
	return position - camera->position;
}

m2::Vec2f m2::camera_to_position_px(const Vec2f& position) {
	return camera_to_position_m(position) * GAME.game_ppm;
}

m2::Vec2f m2::screen_origin_to_position_px(const Vec2f& position) {
	return camera_to_position_px(position) + Vec2f{GAME.windowRect.w / 2, GAME.windowRect.h / 2 };
}

m2::Graphic::Graphic(Id object_id) : Component(object_id) {}
m2::Graphic::Graphic(uint64_t object_id, const Sprite& sprite) : Component(object_id), sprite(&sprite), on_draw(default_draw) {}
m2::Object& m2::Graphic::parent() const {
	return *GAME.objects.get(object_id);
}

m2::Vec2f m2::Graphic::sprite_center_to_sprite_origin_px() const {
	if (sprite) {
		Vec2f vector_in_source_pixels;
		if (draw_foreground_companion) {
			vector_in_source_pixels = Vec2f{sprite->sprite().foreground_companion().center_offset_px()};
		} else {
			vector_in_source_pixels = Vec2f{sprite->sprite().center_offset_px()};
		}

		auto [mul, div] = GAME.pixel_scale_mul_div(sprite->ppm());
		auto vector_in_destination_pixels = vector_in_source_pixels * (float)mul / (float)div;
		return vector_in_destination_pixels;
	} else {
		return {};
	}
}

m2::Vec2f m2::Graphic::screen_origin_to_sprite_center_px() const {
	return screen_origin_to_position_px(parent().position) - sprite_center_to_sprite_origin_px();
}
void m2::Graphic::default_draw(Graphic& gfx) {
	if (not gfx.sprite) {
		// This function only draws sprites
		return;
	}

	auto* texture = gfx.sprite->sprite_sheet().texture();
	if (gfx.draw_effect_type) {
		texture = gfx.sprite->effects_texture();
	} else if (gfx.draw_foreground_companion) {
		texture = gfx.sprite->foreground_companions_texture();
	}

	auto src_rect = sdl::to_rect(gfx.sprite->sprite().rect());
	if (gfx.draw_effect_type) {
		src_rect = gfx.sprite->effect_rect(gfx.draw_effect_type);
	} else if (gfx.draw_foreground_companion) {
		src_rect = gfx.sprite->foreground_companion_rect();
	}

	auto screen_origin_to_sprite_center_px = gfx.screen_origin_to_sprite_center_px();
	auto [mul, div] = GAME.pixel_scale_mul_div(gfx.sprite->ppm());
	auto dst_rect = SDL_Rect{
		(int)roundf(screen_origin_to_sprite_center_px.x) - (src_rect.w * mul / div / 2),
		(int)roundf(screen_origin_to_sprite_center_px.y) - (src_rect.h * mul / div / 2),
		src_rect.w * mul / div,
		src_rect.h * mul / div
	};

	auto centerPoint = SDL_Point{
		(int)roundf(gfx.sprite_center_to_sprite_origin_px().x) + dst_rect.w / 2 ,
		(int)roundf(gfx.sprite_center_to_sprite_origin_px().y) + dst_rect.h / 2
	};

	if (SDL_RenderCopyEx(GAME.sdlRenderer, texture, &src_rect, &dst_rect, gfx.draw_angle * 180.0f / PI, &centerPoint, SDL_FLIP_NONE)) {
		throw M2ERROR("SDL error while drawing: " + std::string(SDL_GetError()));
	}
}
void m2::Graphic::default_draw_healthbar(Graphic& gfx, float healthRatio) {
	auto& obj = GAME.objects[gfx.object_id];
	auto& cam = GAME.objects[GAME.cameraId];

	auto src_rect = sdl::to_rect(gfx.sprite->sprite().rect());
	auto center_offset_px = Vec2f{gfx.sprite->sprite().center_offset_px()};
	auto ppm = static_cast<float>(gfx.sprite->ppm());

	Vec2f obj_origin_wrt_camera_obj = obj.position - cam.position;
	Vec2i obj_origin_wrt_screen_center = Vec2i(obj_origin_wrt_camera_obj * GAME.game_ppm);
	auto [mul, div] = GAME.pixel_scale_mul_div(gfx.sprite->ppm());
	Vec2i obj_gfx_origin_wrt_screen_center = obj_origin_wrt_screen_center + Vec2i{
		-(int)roundf(center_offset_px.x) * mul / div,
		-(int)roundf(center_offset_px.y) * mul / div
	};
	Vec2i obj_gfx_origin_wrt_screen_origin = Vec2i{GAME.windowRect.w / 2, GAME.windowRect.h / 2 } + obj_gfx_origin_wrt_screen_center;
	auto obj_gfx_dstrect = SDL_Rect{
		obj_gfx_origin_wrt_screen_origin.x - (src_rect.w * mul / div / 2),
		obj_gfx_origin_wrt_screen_origin.y - (src_rect.h * mul / div / 2),
		src_rect.w * mul / div,
		src_rect.h * mul / div
	};

	int healthBarWidth = obj_gfx_dstrect.w * 8 / 10;

	auto filled_dstrect = SDL_Rect{
		obj_gfx_dstrect.x + (obj_gfx_dstrect.w - healthBarWidth) / 2,
		obj_gfx_dstrect.y + obj_gfx_dstrect.h,
		(int)roundf((float)healthBarWidth * healthRatio),
		(int)48 / 6
	};
	SDL_SetRenderDrawColor(GAME.sdlRenderer, 255, 0, 0, 200);
	SDL_RenderFillRect(GAME.sdlRenderer, &filled_dstrect);

	auto empty_dstrect = SDL_Rect{
		filled_dstrect.x + filled_dstrect.w,
		filled_dstrect.y,
		healthBarWidth - filled_dstrect.w,
		filled_dstrect.h
	};
	SDL_SetRenderDrawColor(GAME.sdlRenderer, 127, 0, 0, 200);
	SDL_RenderFillRect(GAME.sdlRenderer, &empty_dstrect);
}
