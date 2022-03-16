#include <m2/Component.h>
#include <m2/Object.h>
#include <m2/Game.h>
#include <m2/Def.h>

Vec2I ComponentGraphic_GraphicsOriginWRTScreenCenter_px(Vec2F objPosition, Vec2F objGfxCenterPx) {
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
	Vec2I obj_gfx_origin_wrt_screen_center_px = Vec2I_Add(obj_origin_wrt_screen_center_px, (Vec2I) {
			-(int)roundf(objGfxCenterPx.x * GAME->scale),
			-(int)roundf(objGfxCenterPx.y * GAME->scale)
	});
	return obj_gfx_origin_wrt_screen_center_px;
}

void ComponentGraphic_DefaultDraw(ComponentGraphic* gfx) {
	Object* obj = Pool_GetById(&GAME->objects, gfx->super.objId); M2ASSERT(obj);

	Vec2I obj_gfx_origin_wrt_screen_center_px = ComponentGraphic_GraphicsOriginWRTScreenCenter_px(obj->position, gfx->center_px);
	// Screen origin is top-left corner
	Vec2I obj_gfx_origin_wrt_screen_origin_px = Vec2I_Add((Vec2I) { GAME->windowRect.w / 2, GAME->windowRect.h / 2 }, obj_gfx_origin_wrt_screen_center_px);
	SDL_Rect dstrect = (SDL_Rect){
			obj_gfx_origin_wrt_screen_origin_px.x - (int)floorf((float)gfx->textureRect.w * GAME->scale / 2.0f),
			obj_gfx_origin_wrt_screen_origin_px.y - (int)floorf((float)gfx->textureRect.h * GAME->scale / 2.0f),
			(int)ceilf((float)gfx->textureRect.w * GAME->scale),
			(int)ceilf((float)gfx->textureRect.h * GAME->scale)
	};
	SDL_Point centerPoint = (SDL_Point){
			(int)roundf(gfx->center_px.x * GAME->scale) + dstrect.w/2 ,
			(int)roundf(gfx->center_px.y * GAME->scale) + dstrect.h/2
	};
	SDL_RenderCopyEx(GAME->sdlRenderer, gfx->texture, &gfx->textureRect, &dstrect, gfx->angle * 180.0 / M2_PI, &centerPoint, SDL_FLIP_NONE);
}

void ComponentGraphic_DefaultDrawHealthBar(ComponentGraphic* gfx, float healthRatio) {
	Object* obj = Game_FindObjectById(gfx->super.objId); M2ASSERT(obj);
	Object* camera = Pool_GetById(&GAME->objects, GAME->cameraId); M2ASSERT(camera);

	Vec2F obj_origin_wrt_camera_obj = Vec2F_Sub(obj->position, camera->position);
	Vec2I obj_origin_wrt_screen_center = Vec2I_From2F(Vec2F_Mul(obj_origin_wrt_camera_obj, GAME->pixelsPerMeter));
	Vec2I obj_gfx_origin_wrt_screen_center = Vec2I_Add(obj_origin_wrt_screen_center, (Vec2I) {
		-(int)roundf(gfx->center_px.x * GAME->scale),
		-(int)roundf(gfx->center_px.y * GAME->scale)
	});
	Vec2I obj_gfx_origin_wrt_screen_origin = Vec2I_Add((Vec2I) { GAME->windowRect.w / 2, GAME->windowRect.h / 2 }, obj_gfx_origin_wrt_screen_center);
	SDL_Rect obj_gfx_dstrect = (SDL_Rect){
		obj_gfx_origin_wrt_screen_origin.x - (int)roundf((float)gfx->textureRect.w * GAME->scale / 2.0f),
		obj_gfx_origin_wrt_screen_origin.y - (int)roundf((float)gfx->textureRect.h * GAME->scale / 2.0f),
		(int)roundf((float)gfx->textureRect.w * GAME->scale),
		(int)roundf((float)gfx->textureRect.h * GAME->scale)
	};

	int healthBarWidth = obj_gfx_dstrect.w * 8 / 10;

	SDL_Rect filled_dstrect = (SDL_Rect){
		obj_gfx_dstrect.x + (obj_gfx_dstrect.w - healthBarWidth) / 2,
		obj_gfx_dstrect.y + obj_gfx_dstrect.h,
		(int)roundf((float)healthBarWidth * healthRatio),
		GAME->proxy.tileSize / 6
	};
	SDL_SetRenderDrawColor(GAME->sdlRenderer, 255, 0, 0, 200);
	SDL_RenderFillRect(GAME->sdlRenderer, &filled_dstrect);

	SDL_Rect empty_dstrect = (SDL_Rect){
		filled_dstrect.x + filled_dstrect.w,
		filled_dstrect.y,
		healthBarWidth - filled_dstrect.w,
		filled_dstrect.h
	};
	SDL_SetRenderDrawColor(GAME->sdlRenderer, 127, 0, 0, 200);
	SDL_RenderFillRect(GAME->sdlRenderer, &empty_dstrect);
}

M2Err ComponentGraphic_Init(ComponentGraphic* gfx, ID objectId) {
	memset(gfx, 0, sizeof(ComponentGraphic));
	M2ERR_REFLECT(Component_Init((Component*)gfx, objectId));
	gfx->texture = GAME->sdlTexture;
	gfx->draw = ComponentGraphic_DefaultDraw;
	return M2OK;
}

void ComponentGraphic_Term(ComponentGraphic* gfx) {
	Component_Term((Component*)gfx);
	memset(gfx, 0, sizeof(ComponentGraphic));
}

int ComponentGraphic_YComparatorCB(ID gfxIdA, ID gfxIdB) {
	ComponentGraphic* gfxA = Pool_GetById(&GAME->graphics, gfxIdA); M2ASSERT(gfxA);
	ComponentGraphic* gfxB = Pool_GetById(&GAME->graphics, gfxIdB); M2ASSERT(gfxB);

	Object* a = Pool_GetById(&GAME->objects, gfxA->super.objId); M2ASSERT(a);
	Object* b = Pool_GetById(&GAME->objects, gfxB->super.objId); M2ASSERT(b);

	float diff = b->position.y - a->position.y;
	if (0 < diff) {
		return 1;
	} else if (diff < 0) {
		return -1;
	} else {
		return 0;
	}
}
