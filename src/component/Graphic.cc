#include <m2/Component.h>
#include <m2/Game.hh>
#include "m2/component/Graphic.h"
#include <m2g/SpriteBlueprint.h>
#include <m2/Object.h>
#include <m2/Game.hh>

m2::Vec2f m2::offset_from_camera_m(Vec2f position) {
	auto* camera = GAME.objects.get(GAME.cameraId);
	return position - camera->position;
}

m2::Vec2i m2::offset_from_camera_px(Vec2f position) {
	return m2::Vec2i{m2::offset_from_camera_m(position) * GAME.game_ppm};
}

m2::comp::Graphic::Graphic(Id object_id) : Component(object_id), texture(GAME.sdlTexture), ppm(GAME.game_ppm), on_draw(default_draw) {}

m2::comp::Graphic::Graphic(uint64_t object_id, const Sprite& sprite) : Component(object_id), on_draw(default_draw) {
	set_sprite(sprite);
}

void m2::comp::Graphic::set_sprite(const Sprite &sprite) {
	texture = sprite.sprite_sheet().texture();
	textureRect = sdl::to_rect(sprite.sprite().rect());
	center_px = Vec2f{sprite.sprite().center_offset_px()};
	ppm = sprite.ppm();
}

m2::Object& m2::comp::Graphic::parent() const {
	return *GAME.objects.get(object_id);
}

m2::Vec2i m2::comp::Graphic::offset_from_screen_center_px() const {
	auto& obj = parent();
	return m2::offset_from_camera_px(obj.position) + Vec2i{
			-(int)roundf(center_px.x * GAME.pixel_scale(ppm)),
			-(int)roundf(center_px.y * GAME.pixel_scale(ppm))
	};
}

m2::Vec2i m2::comp::Graphic::offset_from_screen_origin_px() const {
	return offset_from_screen_center_px() + Vec2i{GAME.windowRect.w / 2, GAME.windowRect.h / 2 };
}

void m2::comp::Graphic::default_draw(comp::Graphic& gfx) {
	auto offset_from_screen_origin_px = gfx.offset_from_screen_origin_px();
	auto pixel_scale = GAME.pixel_scale(gfx.ppm);
	auto dstrect = SDL_Rect{
			offset_from_screen_origin_px.x - (int)floorf((float)gfx.textureRect.w * pixel_scale / 2.0f),
			offset_from_screen_origin_px.y - (int)floorf((float)gfx.textureRect.h * pixel_scale / 2.0f),
			(int)ceilf((float)gfx.textureRect.w * pixel_scale),
			(int)ceilf((float)gfx.textureRect.h * pixel_scale)
	};
	auto centerPoint = SDL_Point{
			(int)roundf(gfx.center_px.x * pixel_scale) + dstrect.w/2 ,
			(int)roundf(gfx.center_px.y * pixel_scale) + dstrect.h/2
	};
	if (SDL_RenderCopyEx(GAME.sdlRenderer, gfx.texture, &gfx.textureRect, &dstrect, gfx.angle * 180.0 / PI, &centerPoint, SDL_FLIP_NONE)) {
		throw M2ERROR("SDL error while drawing: " + std::string(SDL_GetError()));
	}
}

void m2::comp::Graphic::default_draw_healthbar(comp::Graphic& gfx, float healthRatio) {
	auto& obj = GAME.objects[gfx.object_id];
	auto& cam = GAME.objects[GAME.cameraId];

	Vec2f obj_origin_wrt_camera_obj = obj.position - cam.position;
	Vec2i obj_origin_wrt_screen_center = Vec2i(obj_origin_wrt_camera_obj * GAME.game_ppm);
	auto pixel_scale = GAME.pixel_scale(gfx.ppm);
	Vec2i obj_gfx_origin_wrt_screen_center = obj_origin_wrt_screen_center + Vec2i{
			-(int)roundf(gfx.center_px.x * pixel_scale),
			-(int)roundf(gfx.center_px.y * pixel_scale)
	};
	Vec2i obj_gfx_origin_wrt_screen_origin = Vec2i{GAME.windowRect.w / 2, GAME.windowRect.h / 2 } + obj_gfx_origin_wrt_screen_center;
	auto obj_gfx_dstrect = SDL_Rect{
			obj_gfx_origin_wrt_screen_origin.x - (int)roundf((float)gfx.textureRect.w * pixel_scale / 2.0f),
			obj_gfx_origin_wrt_screen_origin.y - (int)roundf((float)gfx.textureRect.h * pixel_scale / 2.0f),
			(int)roundf((float)gfx.textureRect.w * pixel_scale),
			(int)roundf((float)gfx.textureRect.h * pixel_scale)
	};

	int healthBarWidth = obj_gfx_dstrect.w * 8 / 10;

	auto filled_dstrect = SDL_Rect{
			obj_gfx_dstrect.x + (obj_gfx_dstrect.w - healthBarWidth) / 2,
			obj_gfx_dstrect.y + obj_gfx_dstrect.h,
			(int)roundf((float)healthBarWidth * healthRatio),
			(int)m2g::tile_width / 6
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
