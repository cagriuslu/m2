#include "../Component.h"
#include "../Object.h"
#include "../Game.h"
#include "../Def.h"

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

Vec2I ComponentLight_ObjectOriginWRTScreenCenter_px(Vec2F objPosition) {
	static ID cameraId = 0;
	static Object* cameraObj = NULL;
	if (GAME->cameraId && cameraId != GAME->cameraId) {
		cameraId = GAME->cameraId;
		cameraObj = Pool_GetById(&GAME->objects, GAME->cameraId); M2ASSERT(cameraObj);
	}
	Vec2F cameraPosition = cameraObj ? cameraObj->position : VEC2F_ZERO; // cameraObj is NULL while level is loading
	Vec2F obj_origin_wrt_camera_obj_m = Vec2F_Sub(objPosition, cameraPosition);
	// Screen center is the middle of the window
	Vec2I obj_origin_wrt_screen_center_px = Vec2I_From2F(Vec2F_Mul(obj_origin_wrt_camera_obj_m, GAME->pixelsPerMeter));
	return obj_origin_wrt_screen_center_px;
}

void ComponentLight_DefaultDraw(ComponentLight* lig) {
	Object* obj = Pool_GetById(&GAME->objects, lig->super.objId); M2ASSERT(obj);

	Vec2I obj_origin_wrt_screen_center_px = ComponentLight_ObjectOriginWRTScreenCenter_px(obj->position);
	// Screen origin is top-left corner
	Vec2I obj_origin_wrt_screen_origin_px = Vec2I_Add((Vec2I) { GAME->windowRect.w / 2, GAME->windowRect.h / 2 }, obj_origin_wrt_screen_center_px);
	SDL_Rect dstrect = (SDL_Rect){
			obj_origin_wrt_screen_origin_px.x - (int)roundf((float)lig->radius_m * GAME->pixelsPerMeter),
			obj_origin_wrt_screen_origin_px.y - (int)roundf((float)lig->radius_m * GAME->pixelsPerMeter),
			(int)roundf((float)lig->radius_m * GAME->pixelsPerMeter * 2.0f),
			(int)roundf((float)lig->radius_m * GAME->pixelsPerMeter * 2.0f)
	};
	SDL_RenderCopy(GAME->sdlRenderer, GAME->sdlLightTexture, NULL, &dstrect);
}
