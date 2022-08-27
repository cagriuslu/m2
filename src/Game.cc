#include "m2/Game.hh"
#include <m2/Log.h>
#include <m2/Exception.h>
#include <m2/Object.h>
#include <m2/object/God.h>
#include <m2/object/Camera.h>
#include <m2/object/Pointer.h>
#include <m2/object/Tile.h>
#include <m2/String.h>
#include <m2/Proto.h>
#include <LevelBlueprint.pb.h>
#include "m2/Component.h"
#include <m2g/SpriteBlueprint.h>
#include <m2/VSON.hh>
#include <m2g/ui/UI.h>
#include "m2/component/Monitor.h"
#include "m2/Component.h"
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include <m2g/SpriteBlueprint.h>
#include <m2/ui/Editor.h>
#include <m2/ui/UI.h>

// Initialize with default values
m2::Game GAME = {
	.physicsStep_s = 1.0f / 80.0f,
	.velocityIterations = 8,
	.positionIterations = 3
};

m2::Level::Level() : type(Type::GAME) {
	// TODO
}

m2::Level::Level(const std::string& path) : type(Type::EDITOR), editor_file_path(path),
	editor_mode(EditorMode::NONE), editor_draw_sprite_index(0), editor_grid_lines(false) {}

m2::Game::~Game() {
    // Get rid of lvl
    objects.clear();
    delete contactListener;
    contactListener = nullptr;
    delete world;
    world = nullptr;
    // TODO deinit others
}

m2::VoidValue m2::Game::load_level(const std::string& level_resource_path) {
	auto lb = proto::json_file_to_message<model::LevelBlueprint>(level_resource_path);
	m2_reflect_failure(lb);
	
	if (level) {
		unload_level();
	}
	level = Level{};

	// Reset state
	events.clear();
	is_phy_stepping = false;
	GAME.world = new b2World(b2Vec2{0.0f, 0.0f});
	GAME.contactListener = new m2::box2d::ContactListener(m2::comp::Physique::contact_cb);
	GAME.world->SetContactListener(GAME.contactListener);

	// Load objects
	for (unsigned y = 0; y < lb->height(); y++) {
		for (unsigned x = 0; x < lb->width(); x++) {
			auto tb = lb->tiles(y * lb->width() + x);
			if (tb.bg_sprite_index()) {
				m2::obj::create_tile(m2::Vec2f{x, y}, tb.bg_sprite_index());
			}
			if (tb.fg_sprite_index()) {
				auto& obj = GAME.objects.alloc().first;
				auto load_result = m2g::fg_sprite_loader(obj, tb.fg_sprite_index(), tb.fg_object_group(), m2::Vec2f{x, y});
				if (!load_result) {
					return failure(load_result.error());
				}
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
	GAME.leftHudUIState->update_positions(GAME.leftHudRect);
	GAME.leftHudUIState->update_contents();
	GAME.rightHudUIState = m2::ui::UIState(&m2g::ui::right_hud);
	GAME.rightHudUIState->update_positions(GAME.rightHudRect);
	GAME.rightHudUIState->update_contents();

	return {};
}

m2::VoidValue m2::Game::load_editor(const std::string& level_resource_path) {
	auto lb = proto::json_file_to_message<model::LevelBlueprint>(level_resource_path);
	m2_reflect_failure(lb);

	if (level) {
		unload_level();
	}
	level = Level{level_resource_path};

	// Reset state
	events.clear();
	is_phy_stepping = false;

	for (unsigned y = 0; y < lb->height(); ++y) {
		for (unsigned x = 0; x < lb->width(); ++x) {
			auto tb = lb->tiles(y * lb->width() + x);
			if (tb.bg_sprite_index()) {
				// Create background tile
				m2::obj::create_tile(m2::Vec2f{x, y}, tb.bg_sprite_index());
			}
			if (tb.fg_sprite_index()) {
				// Create object
				auto& obj = GAME.objects.alloc().first;
				m2_reflect_failure(m2g::fg_sprite_loader(obj, tb.fg_sprite_index(), tb.fg_object_group(), m2::Vec2f{x, y}));
			}
		}
	}

	// Create default objects
	m2::obj::create_god();
	m2::obj::create_camera();
	m2::obj::create_pointer();

	// UI Hud
	GAME.leftHudUIState = m2::ui::UIState(&ui::editor_left_hud);
	GAME.leftHudUIState->update_positions(GAME.leftHudRect);
	GAME.leftHudUIState->update_contents();
	GAME.rightHudUIState = m2::ui::UIState(&ui::editor_right_hud);
	GAME.rightHudUIState->update_positions(GAME.rightHudRect);
	GAME.rightHudUIState->update_contents();

	return {};
}

void m2::Game::unload_level() {
	leftHudUIState = {};
	rightHudUIState = {};

	PathfinderMap_Term(&pathfinderMap);
	offenses.clear();
	defenses.clear();
	lights.clear();
	terrainGraphics.clear();
	graphics.clear();
	physics.clear();
	monitors.clear();
	objects.clear();

	// Check if groups are empty, as it should be
	if (not groups.empty()) {
		LOG_WARN("Level unloaded but not all groups are erased");
	}
	groups.clear();

	delete contactListener;
	contactListener = nullptr;
	delete world;
	world = nullptr;
	level = {};
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
	pixelsPerMeter = (float) gameAndHudRect.h / tilesOnScreen;
	scale = pixelsPerMeter / m2g::tile_width;
}

void m2::Game::update_mouse_position() {
	auto& cam = objects[cameraId];
	m2::Vec2f cameraPosition = cam.position;

	m2::Vec2i pointerPosition = events.mouse_position();
	m2::Vec2i pointerPositionWRTScreenCenter_px = m2::Vec2i{pointerPosition.x - (windowRect.w / 2), pointerPosition.y - (windowRect.h / 2) };
	mousePositionWRTScreenCenter_m = m2::Vec2f{(float) pointerPositionWRTScreenCenter_px.x / pixelsPerMeter, (float) pointerPositionWRTScreenCenter_px.y / pixelsPerMeter };
	mousePositionWRTGameWorld_m = mousePositionWRTScreenCenter_m + cameraPosition;
}

void m2::Game::add_deferred_action(const std::function<void(void)>& action) {
	level->deferred_actions.push_back(action);
}

void m2::Game::execute_deferred_actions() {
	for (auto& action : level->deferred_actions) {
		action();
	}
	level->deferred_actions.clear();
}
