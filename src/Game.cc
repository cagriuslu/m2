#include "m2/Game.hh"
#include <m2/object/Object.hh>
#include "m2/Component.hh"
#include "m2/Box2D.hh"
#include "m2/Cfg.hh"
#include "m2/UI.hh"
#include "m2/Def.hh"
#include <b2_world.h>
#include <game/game_proxy.hh>

// Initialize with default values
Game GAME = {
	.physicsStep_s = 1.0f / 80.0f,
	.velocityIterations = 8,
	.positionIterations = 3,
    .proxy = game::GAME_PROXY
};

void Game_UpdateWindowDimensions(int width, int height) {
	GAME.windowRect = SDL_Rect{0, 0, width, height};

	auto fw = (float)width;
	auto fh = (float)height;
	float aspectRatioDiff = (fw / fh) - GAME_AND_HUD_ASPECT_RATIO;
	if (0.001f < aspectRatioDiff) {
		// Screen is wider than expected, we need envelope on left & right
		GAME.gameRect.w = (int)roundf(fh * GAME_ASPECT_RATIO);
		GAME.gameRect.h = height;

		GAME.gameAndHudRect.w = (int)roundf(fh * GAME_AND_HUD_ASPECT_RATIO);
		GAME.gameAndHudRect.h = height;

		int envelopeWidth = (width - GAME.gameAndHudRect.w) / 2;
		GAME.firstEnvelopeRect = SDL_Rect{ 0, 0, envelopeWidth, height };
		GAME.secondEnvelopeRect = SDL_Rect{ width - envelopeWidth, 0, envelopeWidth, height };
		GAME.gameAndHudRect.x = envelopeWidth;
		GAME.gameAndHudRect.y = 0;

		int hudWidth = (int)roundf((float)GAME.gameAndHudRect.h * HUD_ASPECT_RATIO);
		GAME.leftHudRect = SDL_Rect{ envelopeWidth, 0, hudWidth, GAME.gameAndHudRect.h };
		GAME.rightHudRect = SDL_Rect{ width - envelopeWidth - hudWidth, 0, hudWidth, GAME.gameAndHudRect.h };
		GAME.gameRect.x = envelopeWidth + hudWidth;
		GAME.gameRect.y = 0;
	} else if (aspectRatioDiff < -0.001f) {
		// Screen is taller than expected, we need envelope on top & bottom
		GAME.gameRect.w = width;
		GAME.gameRect.h = (int)roundf(fw / GAME_ASPECT_RATIO);

		GAME.gameAndHudRect.w = width;
		GAME.gameAndHudRect.h = (int)roundf(fw / GAME_AND_HUD_ASPECT_RATIO);

		int envelopeHeight = (height - GAME.gameAndHudRect.h) / 2;
		GAME.firstEnvelopeRect = SDL_Rect{0, 0, width, envelopeHeight };
		GAME.secondEnvelopeRect = SDL_Rect{0, height - envelopeHeight, width, envelopeHeight };
		GAME.gameAndHudRect.x = 0;
		GAME.gameAndHudRect.y = envelopeHeight;

		int hudWidth = (int)roundf((float)GAME.gameAndHudRect.h * HUD_ASPECT_RATIO);
		GAME.leftHudRect = SDL_Rect{0, envelopeHeight, hudWidth, GAME.gameAndHudRect.h };
		GAME.rightHudRect = SDL_Rect{ width - hudWidth, envelopeHeight, hudWidth, GAME.gameAndHudRect.h };
		GAME.gameRect.x = hudWidth;
		GAME.gameRect.y = envelopeHeight;
	} else {
		GAME.gameRect.w = width;
		GAME.gameRect.h = height;

		GAME.gameAndHudRect.w = width;
		GAME.gameAndHudRect.h = height;

		GAME.firstEnvelopeRect = SDL_Rect{ 0,0,0,0, };
		GAME.secondEnvelopeRect = SDL_Rect{ 0,0,0,0, };
		GAME.gameAndHudRect.x = 0;
		GAME.gameAndHudRect.y = 0;

		int hudWidth = (int)roundf((float)GAME.gameAndHudRect.h * HUD_ASPECT_RATIO);
		GAME.leftHudRect = SDL_Rect{ 0, 0, hudWidth, GAME.gameAndHudRect.h };
		GAME.rightHudRect = SDL_Rect{ width - hudWidth, 0, hudWidth, GAME.gameAndHudRect.h };
		GAME.gameRect.x = hudWidth;
		GAME.gameRect.y = 0;
	}
	GAME.pixelsPerMeter = (float)GAME.gameAndHudRect.h / 16.0f;
	GAME.scale = GAME.pixelsPerMeter / GAME.proxy.tileSize;
}

void Game_UpdateMousePosition() {
	auto& cam = GAME.objects[GAME.cameraId];
	m2::vec2f cameraPosition = cam.position;

	m2::vec2i pointerPosition = GAME.events.mousePosition;
	m2::vec2i pointerPositionWRTScreenCenter_px = m2::vec2i{pointerPosition.x - (GAME.windowRect.w / 2), pointerPosition.y - (GAME.windowRect.h / 2) };
	GAME.mousePositionWRTScreenCenter_m = m2::vec2f{pointerPositionWRTScreenCenter_px.x / GAME.pixelsPerMeter, pointerPositionWRTScreenCenter_px.y / GAME.pixelsPerMeter };
	GAME.mousePositionInWorld_m = GAME.mousePositionWRTScreenCenter_m + cameraPosition;
}

static int Game_Level_Init() {
	GAME.objects.clear();
	M2ERR_REFLECT(InsertionList_Init(&GAME.drawList, UINT16_MAX + 1, ComponentGraphic_YComparatorCB));
	GAME.monitors.clear();
	GAME.physics.clear();
	GAME.graphics.clear();
	GAME.terrainGraphics.clear();
	GAME.lights.clear();
	GAME.defenses.clear();
	GAME.offenses.clear();
	if (b2_version.major != 2 || b2_version.minor != 4 || b2_version.revision != 0) {
		LOG_FATAL("Box2D version mismatch");
		abort();
	}
	GAME.world = new b2World(b2Vec2{0.0f, 0.0f});
	GAME.contactListener = new ContactListener(ComponentPhysique_ContactCB);
	GAME.world->SetContactListener(GAME.contactListener);
	GAME.delete_list.clear();
	GAME.levelLoaded = true;
	return 0;
}

static void Game_Level_Term() {
	// TODO delete members in objects
	PathfinderMap_Term(&GAME.pathfinderMap);
	delete GAME.contactListener;
	delete GAME.world;
	GAME.delete_list.clear();
	GAME.offenses.clear();
	GAME.defenses.clear();
	GAME.lights.clear();
	GAME.terrainGraphics.clear();
	GAME.graphics.clear();
	GAME.physics.clear();
	GAME.monitors.clear();
	InsertionList_Term(&GAME.drawList);
	GAME.objects.clear();
	GAME.levelLoaded = false;
}

M2Err Game_Level_Load(const CfgLevel *cfg) {
	if (GAME.levelLoaded) {
		Game_Level_Term();
	}
	Game_Level_Init();

	for (int y = 0; y < cfg->h; y++) {
		for (int x = 0; x < cfg->w; x++) {
			const CfgTile *cfgTile = cfg->tiles + y * cfg->w + x;
			if (cfgTile->backgroundSpriteIndex) {
                auto& tile = GAME.objects.alloc().first;
				M2ERR_REFLECT(ObjectTile_InitFromCfg(&tile, cfgTile->backgroundSpriteIndex, m2::vec2f{x, y}));
			}
			if (cfgTile->foregroundSpriteIndex) {
                auto& obj = GAME.objects.alloc().first;
				M2ERR_REFLECT(GAME.proxy.foregroundSpriteLoader(&obj, cfgTile->foregroundSpriteIndex, m2::vec2f{x, y}));
			}
		}
	}
	PathfinderMap_Init(&GAME.pathfinderMap);

    auto camera_pair = GAME.objects.alloc();
    GAME.cameraId = camera_pair.second;
	ObjectCamera_Init(&camera_pair.first);

    auto pointer_pair = GAME.objects.alloc();
    GAME.pointerId = pointer_pair.second;
	ObjectPointer_Init(&pointer_pair.first);

	UIState_Init(&GAME.leftHudUIState, GAME.proxy.cfgHUDLeft);
	UIState_UpdatePositions(&GAME.leftHudUIState, GAME.leftHudRect);
	UIState_UpdateElements(&GAME.leftHudUIState);

	UIState_Init(&GAME.rightHudUIState, GAME.proxy.cfgHUDRight);
	UIState_UpdatePositions(&GAME.rightHudUIState, GAME.rightHudRect);
	UIState_UpdateElements(&GAME.rightHudUIState);

	return M2OK;
}

void Game_DeleteList_Add(ID id) {
	GAME.delete_list.emplace_back(id);
}

void Game_DeleteList_DeleteAll() {
	for (auto id : GAME.delete_list) {
        GAME.objects.free(id);
	}
	GAME.delete_list.clear();
}
