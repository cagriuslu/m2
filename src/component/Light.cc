#include <m2/component/Light.h>
#include <m2/Component.h>
#include <m2/Game.hh>
#include <m2/Object.h>

m2::Light::Light(Id object_id) : Component(object_id), radius_m(0.0f), on_draw(default_draw) {}

m2::Vec2i ComponentLight_ObjectOriginWRTScreenCenter_px(m2::Vec2f objPosition) {
	static m2::Id cameraId = 0;
	static m2::Object* cameraObj = nullptr;
	if (GAME.cameraId && cameraId != GAME.cameraId) {
		cameraId = GAME.cameraId;
		cameraObj = GAME.objects.get(GAME.cameraId);
	}
	m2::Vec2f cameraPosition = cameraObj ? cameraObj->position : m2::Vec2f{}; // cameraObj is NULL while lvl is loading
	m2::Vec2f obj_origin_wrt_camera_obj_m = objPosition - cameraPosition;
	// Screen center is the middle of the window
	m2::Vec2i obj_origin_wrt_screen_center_px = m2::Vec2i(obj_origin_wrt_camera_obj_m * GAME.game_ppm);
	return obj_origin_wrt_screen_center_px;
}

void m2::Light::default_draw(Light& lig) {
	auto& obj = GAME.objects[lig.object_id];

	m2::Vec2i obj_origin_wrt_screen_center_px = ComponentLight_ObjectOriginWRTScreenCenter_px(obj.position);
	// Screen origin is top-left corner
	m2::Vec2i obj_origin_wrt_screen_origin_px = m2::Vec2i{GAME.windowRect.w / 2, GAME.windowRect.h / 2 } + obj_origin_wrt_screen_center_px;
	auto dstrect = SDL_Rect{
			obj_origin_wrt_screen_origin_px.x - (int)roundf((float)lig.radius_m * GAME.game_ppm),
			obj_origin_wrt_screen_origin_px.y - (int)roundf((float)lig.radius_m * GAME.game_ppm),
			(int)roundf((float)lig.radius_m * GAME.game_ppm * 2.0f),
			(int)roundf((float)lig.radius_m * GAME.game_ppm * 2.0f)
	};
	//SDL_RenderCopy(GAME.sdlRenderer, GAME.sdlLightTexture, NULL, &dstrect); Temporarily disabled
}
