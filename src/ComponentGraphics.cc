#include "m2/Component.hh"
#include <m2/object/Object.hh>
#include "m2/Game.hh"
#include "m2/Def.hh"

m2::vec2i ComponentGraphic_GraphicsOriginWRTScreenCenter_px(m2::vec2f objPosition, m2::vec2f objGfxCenterPx) {
	static auto camera_id = GAME.cameraId;
	static auto* camera = GAME.objects.get(GAME.cameraId);
	if (camera_id != GAME.cameraId) {
		// Camera changed
		camera_id = GAME.cameraId;
		camera = GAME.objects.get(GAME.cameraId);
	}

	m2::vec2f cameraPosition = camera->position;
	m2::vec2f obj_origin_wrt_camera_obj_m = objPosition - cameraPosition;
	// Screen center is the middle of the window
	m2::vec2i obj_origin_wrt_screen_center_px = m2::vec2i{obj_origin_wrt_camera_obj_m * GAME.pixelsPerMeter};
	m2::vec2i obj_gfx_origin_wrt_screen_center_px = obj_origin_wrt_screen_center_px + m2::vec2i{
			-(int)roundf(objGfxCenterPx.x * GAME.scale),
			-(int)roundf(objGfxCenterPx.y * GAME.scale)
	};
	return obj_gfx_origin_wrt_screen_center_px;
}

void ComponentGraphic_DefaultDraw(ComponentGraphic* gfx) {
	auto& obj = GAME.objects[gfx->super.objId];

	m2::vec2i obj_gfx_origin_wrt_screen_center_px = ComponentGraphic_GraphicsOriginWRTScreenCenter_px(obj.position, gfx->center_px);
	// Screen origin is top-left corner
	m2::vec2i obj_gfx_origin_wrt_screen_origin_px = m2::vec2i{ GAME.windowRect.w / 2, GAME.windowRect.h / 2 } + obj_gfx_origin_wrt_screen_center_px;
	SDL_Rect dstrect = SDL_Rect{
			obj_gfx_origin_wrt_screen_origin_px.x - (int)floorf((float)gfx->textureRect.w * GAME.scale / 2.0f),
			obj_gfx_origin_wrt_screen_origin_px.y - (int)floorf((float)gfx->textureRect.h * GAME.scale / 2.0f),
			(int)ceilf((float)gfx->textureRect.w * GAME.scale),
			(int)ceilf((float)gfx->textureRect.h * GAME.scale)
	};
	SDL_Point centerPoint = SDL_Point{
			(int)roundf(gfx->center_px.x * GAME.scale) + dstrect.w/2 ,
			(int)roundf(gfx->center_px.y * GAME.scale) + dstrect.h/2
	};
	SDL_RenderCopyEx(GAME.sdlRenderer, gfx->texture, &gfx->textureRect, &dstrect, gfx->angle * 180.0 / M2_PI, &centerPoint, SDL_FLIP_NONE);
}

void ComponentGraphic_DefaultDrawHealthBar(ComponentGraphic* gfx, float healthRatio) {
	auto& obj = GAME.objects[gfx->super.objId];
	auto& cam = GAME.objects[GAME.cameraId];

	m2::vec2f obj_origin_wrt_camera_obj = obj.position - cam.position;
	m2::vec2i obj_origin_wrt_screen_center = m2::vec2i(obj_origin_wrt_camera_obj * GAME.pixelsPerMeter);
	m2::vec2i obj_gfx_origin_wrt_screen_center = obj_origin_wrt_screen_center + m2::vec2i{
		-(int)roundf(gfx->center_px.x * GAME.scale),
		-(int)roundf(gfx->center_px.y * GAME.scale)
	};
	m2::vec2i obj_gfx_origin_wrt_screen_origin = m2::vec2i{ GAME.windowRect.w / 2, GAME.windowRect.h / 2 } + obj_gfx_origin_wrt_screen_center;
	SDL_Rect obj_gfx_dstrect = SDL_Rect{
		obj_gfx_origin_wrt_screen_origin.x - (int)roundf((float)gfx->textureRect.w * GAME.scale / 2.0f),
		obj_gfx_origin_wrt_screen_origin.y - (int)roundf((float)gfx->textureRect.h * GAME.scale / 2.0f),
		(int)roundf((float)gfx->textureRect.w * GAME.scale),
		(int)roundf((float)gfx->textureRect.h * GAME.scale)
	};

	int healthBarWidth = obj_gfx_dstrect.w * 8 / 10;

	SDL_Rect filled_dstrect = SDL_Rect{
		obj_gfx_dstrect.x + (obj_gfx_dstrect.w - healthBarWidth) / 2,
		obj_gfx_dstrect.y + obj_gfx_dstrect.h,
		(int)roundf((float)healthBarWidth * healthRatio),
		(int)GAME.proxy.tileSize / 6
	};
	SDL_SetRenderDrawColor(GAME.sdlRenderer, 255, 0, 0, 200);
	SDL_RenderFillRect(GAME.sdlRenderer, &filled_dstrect);

	SDL_Rect empty_dstrect = SDL_Rect{
		filled_dstrect.x + filled_dstrect.w,
		filled_dstrect.y,
		healthBarWidth - filled_dstrect.w,
		filled_dstrect.h
	};
	SDL_SetRenderDrawColor(GAME.sdlRenderer, 127, 0, 0, 200);
	SDL_RenderFillRect(GAME.sdlRenderer, &empty_dstrect);
}

M2Err ComponentGraphic_Init(ComponentGraphic* gfx, ID objectId) {
	memset(gfx, 0, sizeof(ComponentGraphic));
	M2ERR_REFLECT(Component_Init((Component*)gfx, objectId));
	gfx->texture = GAME.sdlTexture;
	gfx->draw = ComponentGraphic_DefaultDraw;

	return M2OK;
}

void ComponentGraphic_Term(ComponentGraphic* gfx) {
	Component_Term((Component*)gfx);
	memset(gfx, 0, sizeof(ComponentGraphic));
}

int ComponentGraphic_YComparatorCB(ID gfxIdA, ID gfxIdB) {
	auto& gfx_a = GAME.graphics[gfxIdA];
	auto& gfx_b = GAME.graphics[gfxIdB];
	auto& obj_a = GAME.objects[gfx_a.super.objId];
	auto& obj_b = GAME.objects[gfx_b.super.objId];
	float diff = obj_b.position.y - obj_a.position.y;
	if (0 < diff) {
		return 1;
	} else if (diff < 0) {
		return -1;
	} else {
		return 0;
	}
}
