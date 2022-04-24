#include "m2/Game.hh"
#include <m2/Object.h>
#include <m2/object/Camera.h>
#include <m2/object/Pointer.h>
#include <m2/object/Tile.h>
#include "m2/Component.h"
#include <m2g/SpriteBlueprint.h>
#include "m2/Def.h"
#include <b2_world.h>
#include <m2g/ui/UI.h>
#include "m2/component/Monitor.h"
#include "m2/Component.h"
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include <m2g/SpriteBlueprint.h>
#include <m2/ui/UI.h>

// Initialize with default values
m2::Game GAME = {
	.physicsStep_s = 1.0f / 80.0f,
	.velocityIterations = 8,
	.positionIterations = 3
};

m2::Game::~Game() {
    // Get rid of lvl
    objects.clear();
    delete contactListener;
    contactListener = nullptr;
    delete world;
    world = nullptr;
    // TODO deinit others
}

m2::Object* m2::Game::player() {
	return objects.get(playerId);
}

void m2::Game::add_deferred_action(const std::function<void(void)>& action) {
	level.deferred_actions.push_back(action);
}

void m2::Game::execute_deferred_actions() {
	for (auto& action : level.deferred_actions) {
		action();
	}
	level.deferred_actions.clear();
}

void m2::Game::dynamic_assert() {
    for (m2::SpriteIndex i = 0; i < m2g::sprite_count; i++) {
        if (m2g::sprites[i].index != i) {
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
		GAME.gameRect.w = (int)roundf(fh * GAME_ASPECT_RATIO);
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
    GAME.console_rect.x = GAME.gameRect.x;
    GAME.console_rect.y = GAME.gameRect.y + GAME.gameRect.h * 22 / 24;
    GAME.console_rect.w = GAME.gameRect.w;
    GAME.console_rect.h = GAME.gameRect.h * 2 / 24;
	GAME.pixelsPerMeter = (float)GAME.gameAndHudRect.h / 16.0f;
	GAME.scale = GAME.pixelsPerMeter / m2g::tile_width;
}

void Game_UpdateMousePosition() {
	auto& cam = GAME.objects[GAME.cameraId];
	m2::Vec2f cameraPosition = cam.position;

	m2::Vec2i pointerPosition = GAME.events.mouse_position();
	m2::Vec2i pointerPositionWRTScreenCenter_px = m2::Vec2i{pointerPosition.x - (GAME.windowRect.w / 2), pointerPosition.y - (GAME.windowRect.h / 2) };
	GAME.mousePositionWRTScreenCenter_m = m2::Vec2f{pointerPositionWRTScreenCenter_px.x / GAME.pixelsPerMeter, pointerPositionWRTScreenCenter_px.y / GAME.pixelsPerMeter };
	GAME.mousePositionInWorld_m = GAME.mousePositionWRTScreenCenter_m + cameraPosition;
}

static int Game_Level_Init() {
	GAME.events.clear();
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
	GAME.contactListener = new m2::box2d::ContactListener(m2::comp::Physique::contact_cb);
	GAME.world->SetContactListener(GAME.contactListener);
	GAME.level.deferred_actions.clear();
	GAME.levelLoaded = true;
	return 0;
}

static void Game_Level_Term() {
	PathfinderMap_Term(&GAME.pathfinderMap);
	GAME.level.deferred_actions.clear();
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

M2Err Game_Level_Load(const m2::LevelBlueprint* blueprint) {
	if (GAME.levelLoaded) {
		Game_Level_Term();
	}
	Game_Level_Init();

	for (unsigned y = 0; y < blueprint->h; y++) {
		for (unsigned x = 0; x < blueprint->w; x++) {
            const m2::TileBlueprint* tile = blueprint->tiles + y * blueprint->w + x;
			if (tile->bg_sprite_index) {
                m2::obj::create_tile(m2::Vec2f{x, y}, tile->bg_sprite_index);
			}
			if (tile->fg_sprite_index) {
                auto& obj = GAME.objects.alloc().first;
				M2ERR_REFLECT(m2g::fg_sprite_loader(obj, tile->fg_sprite_index, m2::Vec2f{x, y}));
			}
		}
	}
	PathfinderMap_Init(&GAME.pathfinderMap);

    m2::obj::create_camera();
    m2::obj::create_pointer();

    GAME.leftHudUIState = m2::ui::UIState(&m2g::ui::left_hud);
    GAME.leftHudUIState.update_positions(GAME.leftHudRect);
    GAME.leftHudUIState.update_contents();

    GAME.rightHudUIState = m2::ui::UIState(&m2g::ui::right_hud);
    GAME.rightHudUIState.update_positions(GAME.rightHudRect);
    GAME.rightHudUIState.update_contents();

	return M2OK;
}
