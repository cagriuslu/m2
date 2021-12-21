#include "Game.h"
#include "Object.h"
#include "Component.h"
#include "Box2D.h"
#include "Cfg.h"
#include "Log.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

Game* gCurrentGame;

void Game_UpdateWindowDimensions(int width, int height) {
	float fw = (float)width;
	float fh = (float)height;
	
	GAME->windowWidth = width;
	GAME->windowHeight = height;
	float aspectRatioDiff = (fw / fh) - GAME_AND_HUD_ASPECT_RATIO;
	if (0.001f < aspectRatioDiff) {
		// Screen is wider than expected, we need envelope on left & right
		GAME->gameWidth = (int)roundf(fh * GAME_ASPECT_RATIO);
		GAME->gameHeight = height;
		GAME->gameAndHudWidth = (int)roundf(fh * GAME_AND_HUD_ASPECT_RATIO);
		GAME->gameAndHudHeight = height;
		int envelopeWidth = (width - GAME->gameAndHudWidth) / 2;
		GAME->firstEnvelopeRect = (SDL_Rect){ 0, 0, envelopeWidth, height };
		GAME->secondEnvelopeRect = (SDL_Rect){ width - envelopeWidth, 0, envelopeWidth, height };
		int hudWidth = (int)roundf((float)GAME->gameAndHudHeight * HUD_ASPECT_RATIO);
		GAME->leftHudRect = (SDL_Rect){ envelopeWidth, 0, hudWidth, GAME->gameAndHudHeight };
		GAME->rightHudRect = (SDL_Rect){ width - envelopeWidth - hudWidth, 0, hudWidth, GAME->gameAndHudHeight };
	} else if (aspectRatioDiff < -0.001f) {
		// Screen is taller than expected, we need envelope on top & bottom
		GAME->gameWidth = width;
		GAME->gameHeight = (int)roundf(fw / GAME_ASPECT_RATIO);
		GAME->gameAndHudWidth = width;
		GAME->gameAndHudHeight = (int)roundf(fw / GAME_AND_HUD_ASPECT_RATIO);
		int envelopeWidth = (height - GAME->gameAndHudHeight) / 2;
		GAME->firstEnvelopeRect = (SDL_Rect){ 0, 0, width, envelopeWidth };
		GAME->secondEnvelopeRect = (SDL_Rect){ 0, height - envelopeWidth, width, envelopeWidth };
		int hudWidth = (int)roundf((float)GAME->gameAndHudHeight * HUD_ASPECT_RATIO);
		GAME->leftHudRect = (SDL_Rect){ 0, envelopeWidth, hudWidth, GAME->gameAndHudHeight };
		GAME->rightHudRect = (SDL_Rect){ width - hudWidth, envelopeWidth, hudWidth, GAME->gameAndHudHeight };
	} else {
		GAME->gameWidth = width;
		GAME->gameHeight = height;
		GAME->gameAndHudWidth = width;
		GAME->gameAndHudHeight = height;
		GAME->firstEnvelopeRect = (SDL_Rect){ 0,0,0,0, };
		GAME->secondEnvelopeRect = (SDL_Rect){ 0,0,0,0, };
		int hudWidth = (int)roundf((float)GAME->gameAndHudHeight * HUD_ASPECT_RATIO);
		GAME->leftHudRect = (SDL_Rect){ 0, 0, hudWidth, GAME->gameAndHudHeight };
		GAME->rightHudRect = (SDL_Rect){ width - hudWidth, 0, hudWidth, GAME->gameAndHudHeight };
	}
	GAME->pixelsPerMeter = (float)GAME->gameAndHudHeight / 16.0f;
}

int Game_Level_Init() {
	if (GAME->levelLoaded) {
		Game_Level_Term(GAME);
	}
	REFLECT_ERROR(Pool_Init(&GAME->objects, 16, sizeof(Object)));
	REFLECT_ERROR(InsertionList_Init(&GAME->drawList, UINT16_MAX + 1, GraphicsComponent_YComparatorCB));
	REFLECT_ERROR(Pool_Init(&GAME->eventListeners, 16, sizeof(ComponentEventListener)));
	REFLECT_ERROR(Pool_Init(&GAME->physics, 16, sizeof(ComponentPhysics)));
	REFLECT_ERROR(Pool_Init(&GAME->graphics, 16, sizeof(ComponentGraphics)));
	REFLECT_ERROR(Pool_Init(&GAME->terrainGraphics, 16, sizeof(ComponentGraphics)));
	REFLECT_ERROR(Pool_Init(&GAME->defenses, 16, sizeof(ComponentDefense)));
	REFLECT_ERROR(Pool_Init(&GAME->offenses, 16, sizeof(ComponentOffense)));
	GAME->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	GAME->contactListener = Box2DContactListenerRegister(PhysicsComponent_ContactCB);
	Box2DWorldSetContactListener(GAME->world, GAME->contactListener);
	REFLECT_ERROR(Array_Init(&GAME->deleteList, sizeof(ID), 16, UINT16_MAX + 1, NULL));
	GAME->levelLoaded = true;
	return 0;
}

XErr Game_Level_Load(const CfgLevel *cfg) {
	for (int y = 0; y < cfg->h; y++) {
		for (int x = 0; x < cfg->w; x++) {
			const CfgLevelTile *lvlTile = cfg->tiles + y * cfg->w + x;
			if (lvlTile->gndTile) {
				Object *tile = Pool_Mark(&GAME->objects, NULL, NULL);
				REFLECT_ERROR(ObjectTile_InitFromCfg(tile, lvlTile->gndTile, VEC2F(x, y)));
			}
			if (lvlTile->chr) {
				Object *obj = Pool_Mark(&GAME->objects, NULL, NULL);
				REFLECT_ERROR(ObjectCharacter_InitFromCfg(obj, lvlTile->chr, VEC2F(x, y)));
			}
		}
	}
	Object* camera = Pool_Mark(&GAME->objects, NULL, &GAME->cameraId);
	ObjectCamera_Init(camera);

	Hud_Init(&GAME->hud);

	return XOK;
}

void Game_Level_DeleteMarkedObjects() {
	for (size_t i = 0; i < GAME->deleteList.length; i++) {
		ID* objIdPtr = Array_Get(&GAME->deleteList, i);
		if (objIdPtr) {
			ID objId = *objIdPtr;
			Object* obj = Pool_GetById(&GAME->objects, objId);
			if (obj) {
				Object_Term(obj);
				Pool_Unmark(&GAME->objects, obj);
			}
		}
	}
	Array_Clear(&GAME->deleteList);
}

void Game_Level_Term() {
	// TODO delete members in objects
	PathfinderMap_Term(&GAME->pathfinderMap);
	Box2DContactListenerDestroy(GAME->contactListener);
	Box2DWorldDestroy(GAME->world);
	Array_Term(&GAME->deleteList);
	Pool_Term(&GAME->offenses);
	Pool_Term(&GAME->defenses);
	Pool_Term(&GAME->terrainGraphics);
	Pool_Term(&GAME->graphics);
	Pool_Term(&GAME->physics);
	Pool_Term(&GAME->eventListeners);
	InsertionList_Term(&GAME->drawList);
	Pool_Term(&GAME->objects);
	GAME->levelLoaded = false;
}

Vec2F CurrentPointerPositionInWorld() {
	Object* camera = Pool_GetById(&GAME->objects, GAME->cameraId);
	Vec2F cameraPosition = camera->position;

	Vec2I pointerPosition = GAME->events.mousePosition;
	Vec2I pointerPositionWRTScreenCenter = (Vec2I){ pointerPosition.x - (GAME->windowWidth / 2), pointerPosition.y - (GAME->windowHeight / 2) };
	Vec2F pointerPositionWRTCameraPos = (Vec2F){ pointerPositionWRTScreenCenter.x / GAME->pixelsPerMeter, pointerPositionWRTScreenCenter.y / GAME->pixelsPerMeter };
	Vec2F pointerPositionWRTWorld = Vec2F_Add(pointerPositionWRTCameraPos, cameraPosition);
	return pointerPositionWRTWorld;
}
