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
#include <m2/Sprite.h>
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
#include <m2/SDLUtils.hh>
#include <SDL_image.h>
#include <m2/ui/Editor.h>
#include <m2/ui/UI.h>

m2::Game* g_game;

m2::Level::Level() : type(Type::SINGLE_PLAYER) {
	// TODO
}

m2::Level::Level(const std::string& path) : type(Type::EDITOR), editor_file_path(path),
	editor_mode(EditorMode::NONE), editor_draw_sprite_index(0), editor_grid_lines(false) {}

m2::Game::Game() {
	update_window_dims(1600, 900); // Store default window dimensions in GAME
	if ((sdlWindow = SDL_CreateWindow("m2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowRect.w, windowRect.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == nullptr) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_SetWindowMinimumSize(sdlWindow, 712, 400);
	SDL_StopTextInput(); // Text input begins activated (sometimes)
	sdlCursor = SDLUtils_CreateCursor();
	SDL_SetCursor(sdlCursor);
	if ((pixelFormat = SDL_GetWindowPixelFormat(sdlWindow)) == SDL_PIXELFORMAT_UNKNOWN) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	if ((sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED)) == nullptr) { // SDL_RENDERER_PRESENTVSYNC
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_Surface* textureMapSurface = IMG_Load(m2g::texture_map_file.data());
	if (not textureMapSurface) {
		throw M2FATAL("SDL error: " + std::string{IMG_GetError()});
	}
	if ((sdlTexture = SDL_CreateTextureFromSurface(sdlRenderer, textureMapSurface)) == nullptr) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	//SDL_SetTextureColorMod(sdlTexture, 127, 127, 127); Temporarily disabled, because lighting is disabled
	SDL_FreeSurface(textureMapSurface);
	SDL_Surface* textureMaskSurface = IMG_Load(m2g::texture_mask_file.data());
	if (textureMaskSurface == nullptr) {
		throw M2FATAL("SDL error: " + std::string{IMG_GetError()});
	}
	if ((sdlTextureMask = SDL_CreateTextureFromSurface(sdlRenderer, textureMaskSurface)) == nullptr) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_FreeSurface(textureMaskSurface);
	SDL_Surface* lightSurface = IMG_Load("resource/RadialGradient-WhiteBlack.png");
	if (lightSurface == nullptr) {
		throw M2FATAL("SDL error: " + std::string{IMG_GetError()});
	}
	if ((sdlLightTexture = SDL_CreateTextureFromSurface(sdlRenderer, lightSurface)) == nullptr) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_FreeSurface(lightSurface);
	SDL_SetTextureBlendMode(sdlLightTexture, SDL_BLENDMODE_MUL);
	SDL_SetTextureAlphaMod(sdlLightTexture, 0);
	SDL_SetTextureColorMod(sdlLightTexture, 127, 127, 127);
	if ((ttfFont = TTF_OpenFont("resource/fonts/perfect_dos_vga_437/Perfect DOS VGA 437.ttf", 32)) == nullptr) {
		throw M2FATAL("SDL error: " + std::string{TTF_GetError()});
	}

	auto [sprite_sheets_tmp, sprites_tmp] = load_sheets_and_sprites(std::string{m2g::sprite_sheets}, sdlRenderer);
	sprite_sheets = std::move(sprite_sheets_tmp);
	sprites = std::move(sprites_tmp);
}

m2::Game::~Game() {
    // Get rid of lvl
    objects.clear();
    delete contactListener;
    contactListener = nullptr;
    delete world;
    world = nullptr;
    // TODO deinit others

	SDL_DestroyRenderer(sdlRenderer);
	SDL_FreeCursor(sdlCursor);
	SDL_DestroyWindow(sdlWindow);
}

m2::VoidValue m2::Game::load_level(const std::string& level_resource_path) {
	auto lb = proto::json_file_to_message<pb::LevelBlueprint>(level_resource_path);
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
	auto lb = proto::json_file_to_message<pb::LevelBlueprint>(level_resource_path);
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
