#include "m2/Game.hh"
#include <m2/Object.h>
#include <m2/object/Camera.h>
#include <m2/object/Pointer.h>
#include <m2/object/Tile.h>
#include "m2/Component.h"
#include <impl/public/SpriteBlueprint.h>
#include "m2/Def.h"
#include <b2_world.h>
#include <impl/public/ui/UI.h>
#include "m2/component/Monitor.h"
#include "m2/Component.h"
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include <impl/public/SpriteBlueprint.h>

// Initialize with default values
m2::Game GAME = {
	.physicsStep_s = 1.0f / 80.0f,
	.velocityIterations = 8,
	.positionIterations = 3
};

m2::Game::~Game() {
    // Get rid of level
    objects.clear();
    delete contactListener;
    contactListener = nullptr;
    delete world;
    world = nullptr;
    // TODO deinit others
}

void m2::Game::dynamic_assert() {
    for (m2::SpriteIndex i = 0; i < impl::sprite_count; i++) {
        if (impl::sprites[i].index != i) {
            throw M2FATAL(M2ERR_DYNAMIC_ASSERT);
        }
    }
}

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
	GAME.scale = GAME.pixelsPerMeter / impl::tile_width;
}

void Game_UpdateMousePosition() {
	auto& cam = GAME.objects[GAME.cameraId];
	m2::Vec2f cameraPosition = cam.position;

	m2::Vec2i pointerPosition = GAME.events.mousePosition;
	m2::Vec2i pointerPositionWRTScreenCenter_px = m2::Vec2i{pointerPosition.x - (GAME.windowRect.w / 2), pointerPosition.y - (GAME.windowRect.h / 2) };
	GAME.mousePositionWRTScreenCenter_m = m2::Vec2f{pointerPositionWRTScreenCenter_px.x / GAME.pixelsPerMeter, pointerPositionWRTScreenCenter_px.y / GAME.pixelsPerMeter };
	GAME.mousePositionInWorld_m = GAME.mousePositionWRTScreenCenter_m + cameraPosition;
}

static int Game_Level_Init() {
	GAME.objects.clear();
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
	GAME.contactListener = new m2::box2d::ContactListener(m2::component::Physique::contact_cb);
	GAME.world->SetContactListener(GAME.contactListener);
	GAME.delete_list.clear();
	GAME.levelLoaded = true;
	return 0;
}

static void Game_Level_Term() {
	PathfinderMap_Term(&GAME.pathfinderMap);
	GAME.delete_list.clear();
	GAME.offenses.clear();
	GAME.defenses.clear();
	GAME.lights.clear();
	GAME.terrainGraphics.clear();
	GAME.graphics.clear();
	GAME.physics.clear();
	GAME.monitors.clear();
	GAME.objects.clear();
    delete GAME.world;
    delete GAME.contactListener;
	GAME.levelLoaded = false;
}

M2Err Game_Level_Load(const m2::LevelBlueprint* cfg) {
	if (GAME.levelLoaded) {
		Game_Level_Term();
	}
	Game_Level_Init();

	for (int y = 0; y < cfg->h; y++) {
		for (int x = 0; x < cfg->w; x++) {
            const m2::TileBlueprint* tile = cfg->tiles + y * cfg->w + x;
			if (tile->bg_sprite_index) {
                m2::object::create_tile(m2::Vec2f{x, y}, tile->bg_sprite_index);
			}
			if (tile->fg_sprite_index) {
                auto& obj = GAME.objects.alloc().first;
				M2ERR_REFLECT(impl::fg_sprite_loader(&obj, tile->fg_sprite_index, m2::Vec2f{x, y}));
			}
		}
	}
	PathfinderMap_Init(&GAME.pathfinderMap);

    m2::object::create_camera();
    m2::object::create_pointer();

    GAME.leftHudUIState = m2::ui::UIState(&impl::ui::left_hud);
    GAME.leftHudUIState.update_positions(GAME.leftHudRect);
    GAME.leftHudUIState.update_contents();

    GAME.rightHudUIState = m2::ui::UIState(&impl::ui::right_hud);
    GAME.rightHudUIState.update_positions(GAME.rightHudRect);
    GAME.rightHudUIState.update_contents();

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
