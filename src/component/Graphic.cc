#include <m2/Component.h>
#include <m2/Game.h>
#include "m2/component/Graphic.h"
#include <m2/Object.h>

m2::Vec2f m2::camera_to_position_m(const Vec2f& position) {
	auto* camera = LEVEL.objects.get(LEVEL.cameraId);
	return position - camera->position;
}

m2::Vec2f m2::camera_to_position_px(const Vec2f& position) {
	return camera_to_position_m(position) * GAME.game_ppm;
}

m2::Vec2f m2::screen_origin_to_position_px(const Vec2f& position) {
	return camera_to_position_px(position) + Vec2f{GAME.windowRect.w / 2, GAME.windowRect.h / 2 };
}

m2::Graphic::Graphic(Id object_id) : Component(object_id) {}
m2::Graphic::Graphic(uint64_t object_id, const Sprite& sprite) : Component(object_id), on_draw(default_draw), on_effect(default_effect), sprite(&sprite) {}

m2::Vec2f m2::Graphic::sprite_center_to_sprite_origin_px() const {
	if (sprite) {
		Vec2f vector_in_source_pixels;
		if (draw_sprite_effect == pb::SPRITE_EFFECT_FOREGROUND_COMPANION && sprite->has_foreground_companion()) {
			vector_in_source_pixels = sprite->foreground_companion_center_offset_px();
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
	if (gfx.draw_sprite_effect) {
		texture = gfx.sprite->effects_texture();
	}

	auto src_rect = sdl::to_rect(gfx.sprite->sprite().rect());
	if (gfx.draw_sprite_effect) {
		src_rect = gfx.sprite->effect_rect(gfx.draw_sprite_effect);
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
void m2::Graphic::default_effect(Graphic& gfx) {
	if (not gfx.sprite) {
		// This function only works if there is a sprite
		return;
	}
	if (not gfx.draw_effect_health_bar) {
		return;
	}

	auto src_rect = sdl::to_rect(gfx.sprite->sprite().rect());

	// White frame
	auto screen_origin_to_sprite_center_px = gfx.screen_origin_to_sprite_center_px();
	auto [mul, div] = GAME.pixel_scale_mul_div(gfx.sprite->ppm());
	auto dst_rect = SDL_Rect{
			(int)roundf(screen_origin_to_sprite_center_px.x) - (src_rect.w * mul / div / 2),
			(int)roundf(screen_origin_to_sprite_center_px.y) + (src_rect.h * mul * 11 / div / 2 / 10), // Give an offset of 1.1
			src_rect.w * mul / div,
			15 * mul / 100 // 0.15 m height
	};
	SDL_SetRenderDrawColor(GAME.sdlRenderer, 255, 255, 255, 255);
	SDL_RenderFillRect(GAME.sdlRenderer, &dst_rect);

	// Black shadow
	auto shadow_rect = SDL_Rect{dst_rect.x + 1, dst_rect.y + 1, dst_rect.w - 2, dst_rect.h - 2};
	SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
	SDL_RenderFillRect(GAME.sdlRenderer, &shadow_rect);

	// Green part
	float percentage = (*gfx.draw_effect_health_bar) < 0.0f ? 0.0f : (1.0f < *gfx.draw_effect_health_bar) ? 1.0f :  *gfx.draw_effect_health_bar;
	SDL_Rect green_rect;
	if (7 <= dst_rect.w) {
		green_rect = SDL_Rect{shadow_rect.x + 1, shadow_rect.y + 1, (int)roundf(percentage * (float)(shadow_rect.w - 2)), shadow_rect.h - 2};
	} else {
		green_rect = SDL_Rect{dst_rect.x + 1, dst_rect.y + 1, (int)roundf(percentage * (float)(dst_rect.w - 2)), dst_rect.h - 2};
	}
	SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 255, 0, 255);
	SDL_RenderFillRect(GAME.sdlRenderer, &green_rect);
}
