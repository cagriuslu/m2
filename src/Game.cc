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

M2Err m2::Game::load_level(const m2::LevelBlueprint *blueprint) {
	if (levelLoaded) {
		unload_level();
	}

	// Reset state
	events.clear();
	if (b2_version.major != 2 || b2_version.minor != 4 || b2_version.revision != 0) {
		LOG_FATAL("Box2D version mismatch");
		abort();
	}
	GAME.world = new b2World(b2Vec2{0.0f, 0.0f});
	GAME.contactListener = new m2::box2d::ContactListener(m2::comp::Physique::contact_cb);
	GAME.world->SetContactListener(GAME.contactListener);

	// Load objects
	for (unsigned y = 0; y < blueprint->h; y++) {
		for (unsigned x = 0; x < blueprint->w; x++) {
			const m2::TileBlueprint* tile = blueprint->tiles + y * blueprint->w + x;
			if (tile->bg_sprite_index) {
				m2::obj::create_tile(m2::Vec2f{x, y}, tile->bg_sprite_index);
			}
			if (tile->fg_sprite_index) {
				auto& obj = GAME.objects.alloc().first;
				M2ERR_REFLECT(m2g::fg_sprite_loader(obj, tile->fg_sprite_index, tile->fg_object_group, m2::Vec2f{x, y}));
			}
		}
	}
	// Init pathfinder map
	PathfinderMap_Init(&GAME.pathfinderMap);

	// Create default objects
	m2::obj::create_camera();
	m2::obj::create_pointer();

	// Init HUD
	GAME.leftHudUIState = m2::ui::UIState(&m2g::ui::left_hud);
	GAME.leftHudUIState.update_positions(GAME.leftHudRect);
	GAME.leftHudUIState.update_contents();
	GAME.rightHudUIState = m2::ui::UIState(&m2g::ui::right_hud);
	GAME.rightHudUIState.update_positions(GAME.rightHudRect);
	GAME.rightHudUIState.update_contents();

	GAME.levelLoaded = true;
	return M2OK;
}

void m2::Game::unload_level() {
	PathfinderMap_Term(&pathfinderMap);
	level.deferred_actions.clear();
	offenses.clear();
	defenses.clear();
	lights.clear();
	terrainGraphics.clear();
	graphics.clear();
	physics.clear();
	monitors.clear();
	groups.clear();
	objects.clear();
	delete world;
	delete contactListener;
	levelLoaded = false;
}

m2::Object* m2::Game::player() {
	return objects.get(playerId);
}

void m2::Game::update_window_dims(int width, int height) {
	windowRect = SDL_Rect{0, 0, width, height};

	auto fw = (float)width;
	auto fh = (float)height;
	float aspectRatioDiff = (fw / fh) - GAME_AND_HUD_ASPECT_RATIO;
	if (0.001f < aspectRatioDiff) {
		// Screen is wider than expected, we need envelope on left & right
		gameRect.w = (int)roundf(fh * GAME_ASPECT_RATIO);
		gameRect.h = height;

		gameAndHudRect.w = (int)roundf(fh * GAME_AND_HUD_ASPECT_RATIO);
		gameAndHudRect.h = height;

		int envelopeWidth = (width - gameAndHudRect.w) / 2;
		firstEnvelopeRect = SDL_Rect{ 0, 0, envelopeWidth, height };
		secondEnvelopeRect = SDL_Rect{ width - envelopeWidth, 0, envelopeWidth, height };
		gameAndHudRect.x = envelopeWidth;
		gameAndHudRect.y = 0;

		int hudWidth = (int)roundf((float) gameAndHudRect.h * HUD_ASPECT_RATIO);
		leftHudRect = SDL_Rect{ envelopeWidth, 0, hudWidth, gameAndHudRect.h };
		rightHudRect = SDL_Rect{ width - envelopeWidth - hudWidth, 0, hudWidth, gameAndHudRect.h };
		gameRect.x = envelopeWidth + hudWidth;
		gameRect.y = 0;
	} else if (aspectRatioDiff < -0.001f) {
		// Screen is taller than expected, we need envelope on top & bottom
		gameRect.w = width;
		gameRect.h = (int)roundf(fw / GAME_ASPECT_RATIO);

		gameAndHudRect.w = width;
		gameAndHudRect.h = (int)roundf(fw / GAME_AND_HUD_ASPECT_RATIO);

		int envelopeHeight = (height - gameAndHudRect.h) / 2;
		firstEnvelopeRect = SDL_Rect{0, 0, width, envelopeHeight };
		secondEnvelopeRect = SDL_Rect{0, height - envelopeHeight, width, envelopeHeight };
		gameAndHudRect.x = 0;
		gameAndHudRect.y = envelopeHeight;

		int hudWidth = (int)roundf((float) gameAndHudRect.h * HUD_ASPECT_RATIO);
		leftHudRect = SDL_Rect{0, envelopeHeight, hudWidth, gameAndHudRect.h };
		rightHudRect = SDL_Rect{ width - hudWidth, envelopeHeight, hudWidth, gameAndHudRect.h };
		gameRect.x = hudWidth;
		gameRect.y = envelopeHeight;
	} else {
		gameRect.w = (int)roundf(fh * GAME_ASPECT_RATIO);
		gameRect.h = height;

		gameAndHudRect.w = width;
		gameAndHudRect.h = height;

		firstEnvelopeRect = SDL_Rect{ 0,0,0,0, };
		secondEnvelopeRect = SDL_Rect{ 0,0,0,0, };
		gameAndHudRect.x = 0;
		gameAndHudRect.y = 0;

		int hudWidth = (int)roundf((float) gameAndHudRect.h * HUD_ASPECT_RATIO);
		leftHudRect = SDL_Rect{ 0, 0, hudWidth, gameAndHudRect.h };
		rightHudRect = SDL_Rect{ width - hudWidth, 0, hudWidth, gameAndHudRect.h };
		gameRect.x = hudWidth;
		gameRect.y = 0;
	}
	console_rect.x = gameRect.x;
	console_rect.y = gameRect.y + gameRect.h * 22 / 24;
	console_rect.w = gameRect.w;
	console_rect.h = gameRect.h * 2 / 24;
	pixelsPerMeter = (float) gameAndHudRect.h / 16.0f;
	scale = pixelsPerMeter / m2g::tile_width;
}

void m2::Game::update_mouse_position() {
	auto& cam = objects[cameraId];
	m2::Vec2f cameraPosition = cam.position;

	m2::Vec2i pointerPosition = events.mouse_position();
	m2::Vec2i pointerPositionWRTScreenCenter_px = m2::Vec2i{pointerPosition.x - (windowRect.w / 2), pointerPosition.y - (windowRect.h / 2) };
	mousePositionWRTScreenCenter_m = m2::Vec2f{(float) pointerPositionWRTScreenCenter_px.x / pixelsPerMeter, (float) pointerPositionWRTScreenCenter_px.y / pixelsPerMeter };
	mousePositionInWorld_m = mousePositionWRTScreenCenter_m + cameraPosition;
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
