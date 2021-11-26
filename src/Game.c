#include "Game.h"
#include "Object.h"
#include "Component.h"
#include "Box2D.h"
#include "TerrainLoader.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

Game* gCurrentGame;

void Game_SetWidthHeight(Game* game, int width, int height) {
	float fw = (float)width;
	float fh = (float)height;
	
	game->windowWidth = width;
	game->windowHeight = height;
	float aspectRatioDiff = (fw / fh) - GAME_AND_HUD_ASPECT_RATIO;
	if (0.001f < aspectRatioDiff) {
		// Screen is wider than expected, we need envelope on left & right
		game->gameWidth = (int)roundf(fh * GAME_ASPECT_RATIO);
		game->gameHeight = height;
		game->gameAndHudWidth = (int)roundf(fh * GAME_AND_HUD_ASPECT_RATIO);
		game->gameAndHudHeight = height;
		int envelopeWidth = (width - game->gameAndHudWidth) / 2;
		game->firstEnvelopeRect = (SDL_Rect){ 0, 0, envelopeWidth, height };
		game->secondEnvelopeRect = (SDL_Rect){ width - envelopeWidth, 0, envelopeWidth, height };
		int hudWidth = (int)roundf((float)game->gameAndHudHeight * HUD_ASPECT_RATIO);
		game->leftHudRect = (SDL_Rect){ envelopeWidth, 0, hudWidth, game->gameAndHudHeight };
		game->rightHudRect = (SDL_Rect){ width - envelopeWidth - hudWidth, 0, hudWidth, game->gameAndHudHeight };
	} else if (aspectRatioDiff < -0.001f) {
		// Screen is taller than expected, we need envelope on top & bottom
		game->gameWidth = width;
		game->gameHeight = (int)roundf(fw / GAME_ASPECT_RATIO);
		game->gameAndHudWidth = width;
		game->gameAndHudHeight = (int)roundf(fw / GAME_AND_HUD_ASPECT_RATIO);
		int envelopeWidth = (height - game->gameAndHudHeight) / 2;
		game->firstEnvelopeRect = (SDL_Rect){ 0, 0, width, envelopeWidth };
		game->secondEnvelopeRect = (SDL_Rect){ 0, height - envelopeWidth, width, envelopeWidth };
		int hudWidth = (int)roundf((float)game->gameAndHudHeight * HUD_ASPECT_RATIO);
		game->leftHudRect = (SDL_Rect){ 0, envelopeWidth, hudWidth, game->gameAndHudHeight };
		game->rightHudRect = (SDL_Rect){ width - hudWidth, envelopeWidth, hudWidth, game->gameAndHudHeight };
	} else {
		game->gameWidth = width;
		game->gameHeight = height;
		game->gameAndHudWidth = width;
		game->gameAndHudHeight = height;
		game->firstEnvelopeRect = (SDL_Rect){ 0,0,0,0, };
		game->secondEnvelopeRect = (SDL_Rect){ 0,0,0,0, };
		int hudWidth = (int)roundf((float)game->gameAndHudHeight * HUD_ASPECT_RATIO);
		game->leftHudRect = (SDL_Rect){ 0, 0, hudWidth, game->gameAndHudHeight };
		game->rightHudRect = (SDL_Rect){ width - hudWidth, 0, hudWidth, game->gameAndHudHeight };
	}
	game->pixelsPerMeter = (float)game->gameAndHudHeight / 16.0f;
}

int Game_Level_Init(Game* game) {
	PROPAGATE_ERROR(Pool_Init(&game->objects, 16, sizeof(Object)));
	PROPAGATE_ERROR(InsertionList_Init(&game->drawList, UINT16_MAX + 1, GraphicsComponent_YComparatorCB));
	PROPAGATE_ERROR(Pool_Init(&game->eventListeners, 16, sizeof(ComponentEventListener)));
	PROPAGATE_ERROR(Pool_Init(&game->physics, 16, sizeof(ComponentPhysics)));
	PROPAGATE_ERROR(Pool_Init(&game->graphics, 16, sizeof(ComponentGraphics)));
	PROPAGATE_ERROR(Pool_Init(&game->terrainGraphics, 16, sizeof(ComponentGraphics)));
	PROPAGATE_ERROR(Pool_Init(&game->defenses, 16, sizeof(ComponentDefense)));
	PROPAGATE_ERROR(Pool_Init(&game->offenses, 16, sizeof(ComponentOffense)));
	game->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	game->contactListener = Box2DContactListenerRegister(PhysicsComponent_ContactCB);
	Box2DWorldSetContactListener(game->world, game->contactListener);
	PROPAGATE_ERROR(Array_Init(&game->deleteList, sizeof(ID), 16, UINT16_MAX + 1, NULL));
	return 0;
}

int Game_Level_LoadTest(Game* game) {
	game->levelType = LEVEL_TYPE_SINGLE_PLAYER;
	
	TerrainLoader_LoadTiles(game, "resources/terrains/test.txt");

	Array standardItemSet;
	Array_Init(&standardItemSet, sizeof(Item), 16, UINT32_MAX, NULL);
	Item_GenerateStandardItemSet(&standardItemSet);
	Character* character = malloc(sizeof(Character));
	assert(character);
	Character_Init(character, CHARTYP_HUMAN, 1, standardItemSet);
	Character_Preprocess(character);

	Object* player = Pool_Mark(&game->objects, NULL, &game->playerId);
	ObjectPlayer_Init(player, character);

	Hud_Init(&game->hud);

	Object* camera = Pool_Mark(&game->objects, NULL, &game->cameraId);
	ObjectCamera_Init(camera);

	const unsigned skeletonCount = 100;
	for (unsigned i = 0; i < skeletonCount; i++) {
		Object* skeleton = Pool_Mark(&game->objects, NULL, NULL);
		ObjectEnemy_Init(skeleton, (Vec2F) { (float)i, -10.0f }, NULL);
	}

	/*Object* wall = Pool_Mark(&game->objects, NULL, NULL);
	ObjectWall_Init(wall, (Vec2F) { 0.0f, -2.0f });

	Object* box = Pool_Mark(&game->objects, NULL, NULL);
	ObjectStaticBox_Init(box, (Vec2F) { -2.0f, 0.0f });*/

	TerrainLoader_LoadEnemies(game, "resources/terrains/test.txt");

	return 0;
}

int Game_Level_LoadEditor(Game* game) {
	game->levelType = LEVEL_TYPE_LEVEL_EDITOR;
	
	TerrainLoader_LoadTiles(game, "resources/terrains/test.txt");

	Object* god = Pool_Mark(&game->objects, NULL, &game->playerId);
	ObjectGod_Init(god); // TODO check return value

	Object* camera = Pool_Mark(&game->objects, NULL, &game->cameraId);
	ObjectCamera_Init(camera);

	Object* skeleton = Pool_Mark(&game->objects, NULL, NULL);
	ObjectEnemy_Init(skeleton, (Vec2F) { -2.0f, -2.0f }, NULL);

	Object* wall = Pool_Mark(&game->objects, NULL, NULL);
	ObjectWall_Init(wall, (Vec2F) { 0.0f, -2.0f });

	Object* box = Pool_Mark(&game->objects, NULL, NULL);
	ObjectStaticBox_Init(box, (Vec2F) { -2.0f, 0.0f });

	return 0;
}

void Game_Level_DeleteMarkedObjects(Game* game) {
	for (size_t i = 0; i < game->deleteList.length; i++) {
		ID* objIdPtr = Array_Get(&game->deleteList, i);
		if (objIdPtr) {
			ID objId = *objIdPtr;
			Object* obj = Pool_GetById(&game->objects, objId);
			if (obj) {
				Object_Term(obj);
				Pool_Unmark(&game->objects, obj);
			}
		}
	}
	Array_Clear(&game->deleteList);
}

void Game_Level_Term(Game* game) {
	// TODO delete members in objects
	PathfinderMap_Term(&game->pathfinderMap);
	Box2DContactListenerDestroy(game->contactListener);
	Box2DWorldDestroy(game->world);
	Array_Term(&game->deleteList);
	Pool_Term(&game->offenses);
	Pool_Term(&game->defenses);
	Pool_Term(&game->terrainGraphics);
	Pool_Term(&game->graphics);
	Pool_Term(&game->physics);
	Pool_Term(&game->eventListeners);
	InsertionList_Term(&game->drawList);
	Pool_Term(&game->objects);
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
