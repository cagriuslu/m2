#include "../Component.h"
#include "../Object.h"
#include "../Game.h"
#include "../Def.h"

#define MOTION_BLUR

static bool IsMotionBlurEnabled() {
#ifdef MOTION_BLUR
	return true;
#else
	return false;
#endif
}

static Vec2I ComponentGraphic_PixelDisplacementToGraphicsOriginWRTScreenCenter(float scale, Vec2F cameraPosition, Vec2F objPosition, Vec2F objGfxCenterPx) {
	Vec2F obj_origin_wrt_camera_obj_m = Vec2F_Sub(objPosition, cameraPosition);
	// Screen center is the middle of the window
	Vec2I obj_origin_wrt_screen_center_px = Vec2I_From2F(Vec2F_Mul(obj_origin_wrt_camera_obj_m, GAME->pixelsPerMeter));
	Vec2I obj_gfx_origin_wrt_screen_center_px = Vec2I_Add(obj_origin_wrt_screen_center_px, (Vec2I) {
			-(int)roundf(objGfxCenterPx.x * scale),
			-(int)roundf(objGfxCenterPx.y * scale)
	});
	return obj_gfx_origin_wrt_screen_center_px;
}

void ComponentGraphic_DefaultDraw(ComponentGraphic* gfx) {
	Object* obj = Pool_GetById(&GAME->objects, gfx->super.objId); XASSERT(obj);
	Object* camera = Pool_GetById(&GAME->objects, GAME->cameraId); XASSERT(camera);

	float scale = GAME->pixelsPerMeter / GAME->tileWidth;
	Vec2I obj_gfx_origin_wrt_screen_center_px = ComponentGraphic_PixelDisplacementToGraphicsOriginWRTScreenCenter(scale, camera->position, obj->position, gfx->center_px);
	if (IsMotionBlurEnabled() == false || !gfx->motionBlurEnabled || Vec2F_IsNan(gfx->prevDrawPosition) || Vec2F_Equals(obj->position, gfx->prevDrawPosition)) {
		// Screen origin is top-left corner
		Vec2I obj_gfx_origin_wrt_screen_origin_px = Vec2I_Add((Vec2I) { GAME->windowRect.w / 2, GAME->windowRect.h / 2 }, obj_gfx_origin_wrt_screen_center_px);
		SDL_Rect dstrect = (SDL_Rect){
				obj_gfx_origin_wrt_screen_origin_px.x - (int)roundf((float)gfx->textureRect.w * scale / 2.0f),
				obj_gfx_origin_wrt_screen_origin_px.y - (int)roundf((float)gfx->textureRect.h * scale / 2.0f),
				(int)roundf((float)gfx->textureRect.w * scale),
				(int)roundf((float)gfx->textureRect.h * scale)
		};
		SDL_Point centerPoint = (SDL_Point){
				(int)roundf(gfx->center_px.x * scale) + dstrect.w/2 ,
				(int)roundf(gfx->center_px.y * scale) + dstrect.h/2
		};
		SDL_RenderCopyEx(GAME->sdlRenderer, GAME->sdlTexture, &gfx->textureRect, &dstrect, gfx->angle * 180.0 / M_PI, &centerPoint, SDL_FLIP_NONE);
	} else {
		// BIG TODO Right now, motion blur is drawn from previous location of the object to the new location of the object
		// Since the camera is also moving, the object should be drawn from previous location of the object on the screen to new location of the object on the screen

		Vec2I prev_obj_gfx_origin_wrt_screen_center_px = ComponentGraphic_PixelDisplacementToGraphicsOriginWRTScreenCenter(scale, camera->position, gfx->prevDrawPosition, gfx->center_px);
		//fprintf(stderr, "prev_obj_gfx_origin_wrt_screen_center_px {.x = %d, .y = %d}\n", prev_obj_gfx_origin_wrt_screen_center_px.x, prev_obj_gfx_origin_wrt_screen_center_px.y);
		Vec2I obj_displacement_on_screen = Vec2I_Sub(obj_gfx_origin_wrt_screen_center_px, prev_obj_gfx_origin_wrt_screen_center_px);
		//fprintf(stderr, "obj_displacement_on_screen {.x = %d, .y = %d}\n", obj_displacement_on_screen.x, obj_displacement_on_screen.y);
		Vec2F obj_displacement_on_screen_f = Vec2F_FromVec2I(obj_displacement_on_screen);
		//fprintf(stderr, "obj_displacement_on_screen_f {.x = %f, .y = %f}\n", obj_displacement_on_screen_f.x, obj_displacement_on_screen_f.y);

		int samplesToDraw = MAX(abs(obj_displacement_on_screen.x), abs(obj_displacement_on_screen.y));
		samplesToDraw = samplesToDraw == 0 ? 1 : samplesToDraw;
		//fprintf(stderr, "samplesToDraw = %d\n", samplesToDraw);
		Vec2F obj_displacement_step = Vec2F_Div(obj_displacement_on_screen_f, (float)samplesToDraw);
		//fprintf(stderr, "obj_displacement_step {.x = %f, .y = %f}\n", obj_displacement_step.x, obj_displacement_step.y);
		float alphaStep = 255.0f / (float)samplesToDraw;
		//fprintf(stderr, "alphaStep = %f\n", alphaStep);
		for (int i = 0; i < samplesToDraw; i++) {
			Vec2F prev_obj_gfx_origin_wrt_screen_center_px_f = Vec2F_FromVec2I(prev_obj_gfx_origin_wrt_screen_center_px);
			//fprintf(stderr, "prev_obj_gfx_origin_wrt_screen_center_px_f {.x = %f, .y = %f}\n", prev_obj_gfx_origin_wrt_screen_center_px_f.x, prev_obj_gfx_origin_wrt_screen_center_px_f.y);
			Vec2F motion_obj_gfx_origin_wrt_screen_center_px_f = Vec2F_Add(prev_obj_gfx_origin_wrt_screen_center_px_f, Vec2F_Mul(obj_displacement_step, (float)i + 1));
			//fprintf(stderr, "motion_obj_gfx_origin_wrt_screen_center_px_f {.x = %f, .y = %f}\n", motion_obj_gfx_origin_wrt_screen_center_px_f.x, motion_obj_gfx_origin_wrt_screen_center_px_f.y);
			Vec2I motion_obj_gfx_origin_wrt_screen_center_px = Vec2I_From2F(motion_obj_gfx_origin_wrt_screen_center_px_f);
			//fprintf(stderr, "motion_obj_gfx_origin_wrt_screen_center_px {.x = %d, .y = %d}\n", motion_obj_gfx_origin_wrt_screen_center_px.x, motion_obj_gfx_origin_wrt_screen_center_px.y);

			// TODO code copied
			// Screen origin is top-left corner
			Vec2I obj_gfx_origin_wrt_screen_origin_px = Vec2I_Add((Vec2I) { GAME->windowRect.w / 2, GAME->windowRect.h / 2 }, motion_obj_gfx_origin_wrt_screen_center_px);
			SDL_Rect dstrect = (SDL_Rect){
					obj_gfx_origin_wrt_screen_origin_px.x - (int)roundf((float)gfx->textureRect.w * scale / 2.0f),
					obj_gfx_origin_wrt_screen_origin_px.y - (int)roundf((float)gfx->textureRect.h * scale / 2.0f),
					(int)roundf((float)gfx->textureRect.w * scale),
					(int)roundf((float)gfx->textureRect.h * scale)
			};
			SDL_Point centerPoint = (SDL_Point){
					(int)roundf(gfx->center_px.x * scale) + dstrect.w/2 ,
					(int)roundf(gfx->center_px.y * scale) + dstrect.h/2
			};
			SDL_SetTextureAlphaMod(GAME->sdlTexture, (uint8_t)roundf(alphaStep * (float)(i + 1)));
			SDL_RenderCopyEx(GAME->sdlRenderer, GAME->sdlTexture, &gfx->textureRect, &dstrect, gfx->angle * 180.0 / M_PI, &centerPoint, SDL_FLIP_NONE);
		}
		SDL_SetTextureAlphaMod(GAME->sdlTexture, 255);
	}
	gfx->prevDrawPosition = obj->position;
}

void ComponentGraphic_DefaultDrawHealthBar(ComponentGraphic* gfx, float healthRatio) {
	Object* obj = Game_FindObjectById(gfx->super.objId); XASSERT(obj);
	Object* camera = Pool_GetById(&GAME->objects, GAME->cameraId); XASSERT(camera);

	float scale = GAME->pixelsPerMeter / GAME->tileWidth;
	Vec2F obj_origin_wrt_camera_obj = Vec2F_Sub(obj->position, camera->position);
	Vec2I obj_origin_wrt_screen_center = Vec2I_From2F(Vec2F_Mul(obj_origin_wrt_camera_obj, GAME->pixelsPerMeter));
	Vec2I obj_gfx_origin_wrt_screen_center = Vec2I_Add(obj_origin_wrt_screen_center, (Vec2I) {
		-(int)roundf(gfx->center_px.x * scale),
		-(int)roundf(gfx->center_px.y * scale)
	});
	Vec2I obj_gfx_origin_wrt_screen_origin = Vec2I_Add((Vec2I) { GAME->windowRect.w / 2, GAME->windowRect.h / 2 }, obj_gfx_origin_wrt_screen_center);
	SDL_Rect obj_gfx_dstrect = (SDL_Rect){
		obj_gfx_origin_wrt_screen_origin.x - (int)roundf((float)gfx->textureRect.w * scale / 2.0f),
		obj_gfx_origin_wrt_screen_origin.y - (int)roundf((float)gfx->textureRect.h * scale / 2.0f),
		(int)roundf((float)gfx->textureRect.w * scale),
		(int)roundf((float)gfx->textureRect.h * scale)
	};

	int healthBarWidth = obj_gfx_dstrect.w * 8 / 10;

	SDL_Rect filled_dstrect = (SDL_Rect){
		obj_gfx_dstrect.x + (obj_gfx_dstrect.w - healthBarWidth) / 2,
		obj_gfx_dstrect.y + obj_gfx_dstrect.h,
		(int)roundf((float)healthBarWidth * healthRatio),
		GAME->tileWidth / 6
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

int ComponentGraphic_Init(ComponentGraphic* gfx, ID objectId) {
	memset(gfx, 0, sizeof(ComponentGraphic));
	XERR_REFLECT(Component_Init((Component*)gfx, objectId));
	gfx->draw = ComponentGraphic_DefaultDraw;
	gfx->prevDrawPosition = VEC2F_NAN;
	gfx->prevDrawAngle = NAN;
	return 0;
}

void ComponentGraphic_Term(ComponentGraphic* gfx) {
	Component_Term((Component*)gfx);
	memset(gfx, 0, sizeof(ComponentGraphic));
}

int ComponentGraphic_YComparatorCB(ID gfxIdA, ID gfxIdB) {
	ComponentGraphic* gfxA = Pool_GetById(&GAME->graphics, gfxIdA); XASSERT(gfxA);
	ComponentGraphic* gfxB = Pool_GetById(&GAME->graphics, gfxIdB); XASSERT(gfxB);

	Object* a = Pool_GetById(&GAME->objects, gfxA->super.objId); XASSERT(a);
	Object* b = Pool_GetById(&GAME->objects, gfxB->super.objId); XASSERT(b);

	float diff = b->position.y - a->position.y;
	if (0 < diff) {
		return 1;
	} else if (diff < 0) {
		return -1;
	} else {
		return 0;
	}
	return 0;
}
