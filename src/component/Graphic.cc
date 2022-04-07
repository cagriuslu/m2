#include <m2/Component.h>
#include <m2/Game.hh>
#include "m2/component/Graphic.h"
#include <impl/public/SpriteBlueprint.h>

m2::Vec2i ComponentGraphic_GraphicsOriginWRTScreenCenter_px(m2::Vec2f objPosition, m2::Vec2f objGfxCenterPx) {
	static auto camera_id = GAME.cameraId;
	static auto* camera = GAME.objects.get(GAME.cameraId);
	if (camera_id != GAME.cameraId) {
		// Camera changed
		camera_id = GAME.cameraId;
		camera = GAME.objects.get(GAME.cameraId);
	}

	m2::Vec2f cameraPosition = camera->position;
	m2::Vec2f obj_origin_wrt_camera_obj_m = objPosition - cameraPosition;
	// Screen center is the middle of the window
	m2::Vec2i obj_origin_wrt_screen_center_px = m2::Vec2i{obj_origin_wrt_camera_obj_m * GAME.pixelsPerMeter};
	m2::Vec2i obj_gfx_origin_wrt_screen_center_px = obj_origin_wrt_screen_center_px + m2::Vec2i{
			-(int)roundf(objGfxCenterPx.x * GAME.scale),
			-(int)roundf(objGfxCenterPx.y * GAME.scale)
	};
	return obj_gfx_origin_wrt_screen_center_px;
}

m2::component::Graphic::Graphic(ID object_id) : Component(object_id), texture(GAME.sdlTexture), textureRect(), center_px(), angle(0.0f), draw(default_draw) {}

void m2::component::Graphic::default_draw(component::Graphic& gfx) {
	auto& obj = GAME.objects[gfx.object_id];

	Vec2i obj_gfx_origin_wrt_screen_center_px = ComponentGraphic_GraphicsOriginWRTScreenCenter_px(obj.position, gfx.center_px);
	// Screen origin is top-left corner
	Vec2i obj_gfx_origin_wrt_screen_origin_px = Vec2i{GAME.windowRect.w / 2, GAME.windowRect.h / 2 } + obj_gfx_origin_wrt_screen_center_px;
	auto dstrect = SDL_Rect{
			obj_gfx_origin_wrt_screen_origin_px.x - (int)floorf((float)gfx.textureRect.w * GAME.scale / 2.0f),
			obj_gfx_origin_wrt_screen_origin_px.y - (int)floorf((float)gfx.textureRect.h * GAME.scale / 2.0f),
			(int)ceilf((float)gfx.textureRect.w * GAME.scale),
			(int)ceilf((float)gfx.textureRect.h * GAME.scale)
	};
	auto centerPoint = SDL_Point{
			(int)roundf(gfx.center_px.x * GAME.scale) + dstrect.w/2 ,
			(int)roundf(gfx.center_px.y * GAME.scale) + dstrect.h/2
	};
	SDL_RenderCopyEx(GAME.sdlRenderer, gfx.texture, &gfx.textureRect, &dstrect, gfx.angle * 180.0 / M2_PI, &centerPoint, SDL_FLIP_NONE);
}

void m2::component::Graphic::default_draw_healthbar(component::Graphic& gfx, float healthRatio) {
	auto& obj = GAME.objects[gfx.object_id];
	auto& cam = GAME.objects[GAME.cameraId];

	Vec2f obj_origin_wrt_camera_obj = obj.position - cam.position;
	Vec2i obj_origin_wrt_screen_center = Vec2i(obj_origin_wrt_camera_obj * GAME.pixelsPerMeter);
	Vec2i obj_gfx_origin_wrt_screen_center = obj_origin_wrt_screen_center + Vec2i{
			-(int)roundf(gfx.center_px.x * GAME.scale),
			-(int)roundf(gfx.center_px.y * GAME.scale)
	};
	Vec2i obj_gfx_origin_wrt_screen_origin = Vec2i{GAME.windowRect.w / 2, GAME.windowRect.h / 2 } + obj_gfx_origin_wrt_screen_center;
	auto obj_gfx_dstrect = SDL_Rect{
			obj_gfx_origin_wrt_screen_origin.x - (int)roundf((float)gfx.textureRect.w * GAME.scale / 2.0f),
			obj_gfx_origin_wrt_screen_origin.y - (int)roundf((float)gfx.textureRect.h * GAME.scale / 2.0f),
			(int)roundf((float)gfx.textureRect.w * GAME.scale),
			(int)roundf((float)gfx.textureRect.h * GAME.scale)
	};

	int healthBarWidth = obj_gfx_dstrect.w * 8 / 10;

	auto filled_dstrect = SDL_Rect{
			obj_gfx_dstrect.x + (obj_gfx_dstrect.w - healthBarWidth) / 2,
			obj_gfx_dstrect.y + obj_gfx_dstrect.h,
			(int)roundf((float)healthBarWidth * healthRatio),
			(int)impl::tile_width / 6
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
