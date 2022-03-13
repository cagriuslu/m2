#include "Game.h"
#include "Object.h"
#include "Component.h"
#include "Box2D.h"
#include "Cfg.h"
#include "UI.h"
#include "Def.h"
#include "Games/GameLauncher.h"

// Initialize with default values
Game gCurrentGame = {
	.physicsStep_s = 1.0f / 80.0f,
	.velocityIterations = 8,
	.positionIterations = 3,
	.proxy = {
		.entryUi = &LAUNCHER_CFG_UI_ENTRYPOINT,
		.entryUiButtonHandler = Launcher_EntryUIButtonHandler,
		.tileSize = 16
	}
};

Game* Game_GetCurrent() {
	return &gCurrentGame;
}

void Game_UpdateWindowDimensions(int width, int height) {
	GAME->windowRect = (SDL_Rect){0, 0, width, height};

	float fw = (float)width;
	float fh = (float)height;
	float aspectRatioDiff = (fw / fh) - GAME_AND_HUD_ASPECT_RATIO;
	if (0.001f < aspectRatioDiff) {
		// Screen is wider than expected, we need envelope on left & right
		GAME->gameRect.w = (int)roundf(fh * GAME_ASPECT_RATIO);
		GAME->gameRect.h = height;

		GAME->gameAndHudRect.w = (int)roundf(fh * GAME_AND_HUD_ASPECT_RATIO);
		GAME->gameAndHudRect.h = height;

		int envelopeWidth = (width - GAME->gameAndHudRect.w) / 2;
		GAME->firstEnvelopeRect = (SDL_Rect){ 0, 0, envelopeWidth, height };
		GAME->secondEnvelopeRect = (SDL_Rect){ width - envelopeWidth, 0, envelopeWidth, height };
		GAME->gameAndHudRect.x = envelopeWidth;
		GAME->gameAndHudRect.y = 0;

		int hudWidth = (int)roundf((float)GAME->gameAndHudRect.h * HUD_ASPECT_RATIO);
		GAME->leftHudRect = (SDL_Rect){ envelopeWidth, 0, hudWidth, GAME->gameAndHudRect.h };
		GAME->rightHudRect = (SDL_Rect){ width - envelopeWidth - hudWidth, 0, hudWidth, GAME->gameAndHudRect.h };
		GAME->gameRect.x = envelopeWidth + hudWidth;
		GAME->gameRect.y = 0;
	} else if (aspectRatioDiff < -0.001f) {
		// Screen is taller than expected, we need envelope on top & bottom
		GAME->gameRect.w = width;
		GAME->gameRect.h = (int)roundf(fw / GAME_ASPECT_RATIO);

		GAME->gameAndHudRect.w = width;
		GAME->gameAndHudRect.h = (int)roundf(fw / GAME_AND_HUD_ASPECT_RATIO);

		int envelopeHeight = (height - GAME->gameAndHudRect.h) / 2;
		GAME->firstEnvelopeRect = (SDL_Rect){0, 0, width, envelopeHeight };
		GAME->secondEnvelopeRect = (SDL_Rect){0, height - envelopeHeight, width, envelopeHeight };
		GAME->gameAndHudRect.x = 0;
		GAME->gameAndHudRect.y = envelopeHeight;

		int hudWidth = (int)roundf((float)GAME->gameAndHudRect.h * HUD_ASPECT_RATIO);
		GAME->leftHudRect = (SDL_Rect){0, envelopeHeight, hudWidth, GAME->gameAndHudRect.h };
		GAME->rightHudRect = (SDL_Rect){ width - hudWidth, envelopeHeight, hudWidth, GAME->gameAndHudRect.h };
		GAME->gameRect.x = hudWidth;
		GAME->gameRect.y = envelopeHeight;
	} else {
		GAME->gameRect.w = width;
		GAME->gameRect.h = height;

		GAME->gameAndHudRect.w = width;
		GAME->gameAndHudRect.h = height;

		GAME->firstEnvelopeRect = (SDL_Rect){ 0,0,0,0, };
		GAME->secondEnvelopeRect = (SDL_Rect){ 0,0,0,0, };
		GAME->gameAndHudRect.x = 0;
		GAME->gameAndHudRect.y = 0;

		int hudWidth = (int)roundf((float)GAME->gameAndHudRect.h * HUD_ASPECT_RATIO);
		GAME->leftHudRect = (SDL_Rect){ 0, 0, hudWidth, GAME->gameAndHudRect.h };
		GAME->rightHudRect = (SDL_Rect){ width - hudWidth, 0, hudWidth, GAME->gameAndHudRect.h };
		GAME->gameRect.x = hudWidth;
		GAME->gameRect.y = 0;
	}
	GAME->pixelsPerMeter = (float)GAME->gameAndHudRect.h / 16.0f;
	GAME->scale = GAME->pixelsPerMeter / GAME->proxy.tileSize;
}

void Game_UpdateMousePosition() {
	Object* camera = Pool_GetById(&GAME->objects, GAME->cameraId);
	Vec2F cameraPosition = camera->position;

	Vec2I pointerPosition = GAME->events.mousePosition;
	Vec2I pointerPositionWRTScreenCenter_px = (Vec2I){pointerPosition.x - (GAME->windowRect.w / 2), pointerPosition.y - (GAME->windowRect.h / 2) };
	GAME->mousePositionWRTScreenCenter_m = (Vec2F){pointerPositionWRTScreenCenter_px.x / GAME->pixelsPerMeter, pointerPositionWRTScreenCenter_px.y / GAME->pixelsPerMeter };
	GAME->mousePositionInWorld_m = Vec2F_Add(GAME->mousePositionWRTScreenCenter_m, cameraPosition);
}

int Game_Level_Init() {
	if (GAME->levelLoaded) {
		Game_Level_Term();
	}
	M2ERR_REFLECT(Pool_Init(&GAME->objects, 16, sizeof(Object)));
	M2ERR_REFLECT(InsertionList_Init(&GAME->drawList, UINT16_MAX + 1, ComponentGraphic_YComparatorCB));
	M2ERR_REFLECT(Pool_Init(&GAME->monitors, 16, sizeof(ComponentMonitor)));
	M2ERR_REFLECT(Pool_Init(&GAME->physics, 16, sizeof(ComponentPhysique)));
	M2ERR_REFLECT(Pool_Init(&GAME->graphics, 16, sizeof(ComponentGraphic)));
	M2ERR_REFLECT(Pool_Init(&GAME->terrainGraphics, 16, sizeof(ComponentGraphic)));
	M2ERR_REFLECT(Pool_Init(&GAME->lights, 16, sizeof(ComponentLight)));
	M2ERR_REFLECT(Pool_Init(&GAME->defenses, 16, sizeof(ComponentDefense)));
	M2ERR_REFLECT(Pool_Init(&GAME->offenses, 16, sizeof(ComponentOffense)));
	GAME->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	GAME->contactListener = Box2DContactListenerRegister(ComponentPhysique_ContactCB);
	Box2DWorldSetContactListener(GAME->world, GAME->contactListener);
	M2ERR_REFLECT(Array_Init(&GAME->deleteList, sizeof(ID), 16, UINT16_MAX + 1, NULL));
	GAME->levelLoaded = true;
	return 0;
}

M2Err Game_Level_Load(const CfgLevel *cfg) {
	for (int y = 0; y < cfg->h; y++) {
		for (int x = 0; x < cfg->w; x++) {
			const CfgLevelTile *lvlTile = cfg->tiles + y * cfg->w + x;
			if (lvlTile->gndTile) {
				Object *tile = Pool_Mark(&GAME->objects, NULL, NULL);
				M2ERR_REFLECT(ObjectTile_InitFromCfg(tile, lvlTile->gndTile, VEC2F(x, y)));
			}
			if (lvlTile->chr) {
				Object *obj = Pool_Mark(&GAME->objects, NULL, NULL);
				M2ERR_REFLECT(ObjectCharacter_InitFromCfg(obj, lvlTile->chr, VEC2F(x, y)));
			}
		}
	}
	Object* camera = Pool_Mark(&GAME->objects, NULL, &GAME->cameraId);
	ObjectCamera_Init(camera);
	Object* pointer = Pool_Mark(&GAME->objects, NULL, &GAME->pointerId);
	ObjectPointer_Init(pointer);

	UIState_Init(&GAME->leftHudUIState, &CFG_UI_HUDLEFT);
	UIState_UpdatePositions(&GAME->leftHudUIState, GAME->leftHudRect);
	UIState_UpdateElements(&GAME->leftHudUIState);

	UIState_Init(&GAME->rightHudUIState, &CFG_UI_HUDRIGHT);
	UIState_UpdatePositions(&GAME->rightHudUIState, GAME->rightHudRect);
	UIState_UpdateElements(&GAME->rightHudUIState);

	return M2OK;
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
	Pool_Term(&GAME->monitors);
	InsertionList_Term(&GAME->drawList);
	Pool_Term(&GAME->objects);
	GAME->levelLoaded = false;
}

Object* Game_FindObjectById(ID id) {
	return Pool_GetById(&GAME->objects, (id));
}

void Game_DeleteList_Add(ID id) {
	Array_Append(&GAME->deleteList, &id);
}

void Game_DeleteList_DeleteAll() {
	for (size_t i = 0; i < GAME->deleteList.length; i++) {
		ID* objIdPtr = Array_Get(&GAME->deleteList, i); M2ASSERT(objIdPtr);
		Object* obj = Pool_GetById(&GAME->objects, *objIdPtr); M2ASSERT(obj);
		Object_Term(obj);
		Pool_Unmark(&GAME->objects, obj);
	}
	Array_Clear(&GAME->deleteList);
}
