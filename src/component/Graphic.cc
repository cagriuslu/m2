#include <m2/Component.h>
#include <m2/Game.hh>
#include "m2/component/Graphic.h"
#include <m2/Object.h>

m2::Vec2f m2::offset_from_camera_m(Vec2f position) {
	auto* camera = GAME.objects.get(GAME.cameraId);
	return position - camera->position;
}

m2::Vec2i m2::offset_from_camera_px(Vec2f position) {
	return m2::Vec2i{m2::offset_from_camera_m(position) * GAME.game_ppm};
}

m2::comp::Graphic::Graphic(Id object_id) : Component(object_id) {}
m2::comp::Graphic::Graphic(uint64_t object_id, const Sprite& sprite) : Component(object_id), sprite(&sprite), on_draw(default_draw) {}
m2::Object& m2::comp::Graphic::parent() const {
	return *GAME.objects.get(object_id);
}
m2::Vec2i m2::comp::Graphic::offset_from_screen_center_px() const {
	auto& obj = parent();

	auto center_offset_px = Vec2f{sprite->sprite().center_offset_px()};
	auto ppm = static_cast<float>(sprite->ppm());

	return m2::offset_from_camera_px(obj.position) + Vec2i{
		-(int)roundf(center_offset_px.x * GAME.pixel_scale(ppm)),
		-(int)roundf(center_offset_px.y * GAME.pixel_scale(ppm))
	};
}
m2::Vec2i m2::comp::Graphic::offset_from_screen_origin_px() const {
	return offset_from_screen_center_px() + Vec2i{GAME.windowRect.w / 2, GAME.windowRect.h / 2 };
}
void m2::comp::Graphic::default_draw(comp::Graphic& gfx) {
	if (not gfx.sprite) {
		return;
	}
	auto* texture = gfx.effect_type == pb::NO_SPRITE_EFFECT ? gfx.sprite->sprite_sheet().texture() : gfx.sprite->effects_texture();
	auto src_rect = gfx.effect_type == pb::NO_SPRITE_EFFECT ? sdl::to_rect(gfx.sprite->sprite().rect()) : gfx.sprite->effect_rect(gfx.effect_type);
	auto center_offset_px = Vec2f{gfx.sprite->sprite().center_offset_px()};
	auto ppm = static_cast<float>(gfx.sprite->ppm());

	auto offset_from_screen_origin_px = gfx.offset_from_screen_origin_px();
	auto pixel_scale = GAME.pixel_scale(ppm);
	auto dst_rect = SDL_Rect{
			offset_from_screen_origin_px.x - (int)floorf((float)src_rect.w * pixel_scale / 2.0f),
			offset_from_screen_origin_px.y - (int)floorf((float)src_rect.h * pixel_scale / 2.0f),
			(int)ceilf((float)src_rect.w * pixel_scale),
			(int)ceilf((float)src_rect.h * pixel_scale)
	};
	auto centerPoint = SDL_Point{
		(int)roundf(center_offset_px.x * pixel_scale) + dst_rect.w / 2 ,
		(int)roundf(center_offset_px.y * pixel_scale) + dst_rect.h / 2
	};
	if (SDL_RenderCopyEx(GAME.sdlRenderer, texture, &src_rect, &dst_rect, gfx.angle * 180.0 / PI, &centerPoint, SDL_FLIP_NONE)) {
		throw M2ERROR("SDL error while drawing: " + std::string(SDL_GetError()));
	}
}
void m2::comp::Graphic::default_draw_healthbar(comp::Graphic& gfx, float healthRatio) {
	auto& obj = GAME.objects[gfx.object_id];
	auto& cam = GAME.objects[GAME.cameraId];

	auto src_rect = sdl::to_rect(gfx.sprite->sprite().rect());
	auto center_offset_px = Vec2f{gfx.sprite->sprite().center_offset_px()};
	auto ppm = static_cast<float>(gfx.sprite->ppm());

	Vec2f obj_origin_wrt_camera_obj = obj.position - cam.position;
	Vec2i obj_origin_wrt_screen_center = Vec2i(obj_origin_wrt_camera_obj * GAME.game_ppm);
	auto pixel_scale = GAME.pixel_scale(ppm);
	Vec2i obj_gfx_origin_wrt_screen_center = obj_origin_wrt_screen_center + Vec2i{
			-(int)roundf(center_offset_px.x * pixel_scale),
			-(int)roundf(center_offset_px.y * pixel_scale)
	};
	Vec2i obj_gfx_origin_wrt_screen_origin = Vec2i{GAME.windowRect.w / 2, GAME.windowRect.h / 2 } + obj_gfx_origin_wrt_screen_center;
	auto obj_gfx_dstrect = SDL_Rect{
			obj_gfx_origin_wrt_screen_origin.x - (int)roundf((float)src_rect.w * pixel_scale / 2.0f),
			obj_gfx_origin_wrt_screen_origin.y - (int)roundf((float)src_rect.h * pixel_scale / 2.0f),
			(int)roundf((float)src_rect.w * pixel_scale),
			(int)roundf((float)src_rect.h * pixel_scale)
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
