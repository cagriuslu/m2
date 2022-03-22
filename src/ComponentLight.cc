#include "m2/Component.hh"
#include <m2/object/Object.hh>
#include "m2/Game.hh"
#include "m2/Def.hh"

M2Err ComponentLight_Init(ComponentLight* lig, ID objectId) {
	memset(lig, 0, sizeof(ComponentLight));
	M2ERR_REFLECT(Component_Init((Component*)lig, objectId));
	lig->draw = ComponentLight_DefaultDraw;
	return M2OK;
}

void ComponentLight_Term(ComponentLight* lig) {
	Component_Term((Component*)lig);
	memset(lig, 0, sizeof(ComponentLight));
}

m2::vec2i ComponentLight_ObjectOriginWRTScreenCenter_px(m2::vec2f objPosition) {
	static ID cameraId = 0;
	static m2::object::Object* cameraObj = NULL;
	if (GAME.cameraId && cameraId != GAME.cameraId) {
		cameraId = GAME.cameraId;
		cameraObj = GAME.objects.get(GAME.cameraId);
	}
	m2::vec2f cameraPosition = cameraObj ? cameraObj->position : m2::vec2f{}; // cameraObj is NULL while level is loading
	m2::vec2f obj_origin_wrt_camera_obj_m = objPosition - cameraPosition;
	// Screen center is the middle of the window
	m2::vec2i obj_origin_wrt_screen_center_px = m2::vec2i(obj_origin_wrt_camera_obj_m * GAME.pixelsPerMeter);
	return obj_origin_wrt_screen_center_px;
}

void ComponentLight_DefaultDraw(ComponentLight* lig) {
	auto& obj = GAME.objects[lig->super.objId];

	m2::vec2i obj_origin_wrt_screen_center_px = ComponentLight_ObjectOriginWRTScreenCenter_px(obj.position);
	// Screen origin is top-left corner
	m2::vec2i obj_origin_wrt_screen_origin_px = m2::vec2i{ GAME.windowRect.w / 2, GAME.windowRect.h / 2 } + obj_origin_wrt_screen_center_px;
	SDL_Rect dstrect = SDL_Rect{
			obj_origin_wrt_screen_origin_px.x - (int)roundf((float)lig->radius_m * GAME.pixelsPerMeter),
			obj_origin_wrt_screen_origin_px.y - (int)roundf((float)lig->radius_m * GAME.pixelsPerMeter),
			(int)roundf((float)lig->radius_m * GAME.pixelsPerMeter * 2.0f),
			(int)roundf((float)lig->radius_m * GAME.pixelsPerMeter * 2.0f)
	};
	SDL_RenderCopy(GAME.sdlRenderer, GAME.sdlLightTexture, NULL, &dstrect);
}
