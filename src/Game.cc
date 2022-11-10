#include "m2/Game.hh"
#include <m2/Log.h>
#include <m2/Glyph.h>
#include <m2/Exception.h>
#include <m2/Object.h>
#include <m2/object/God.h>
#include <m2/object/Camera.h>
#include <m2/object/Origin.h>
#include <m2/object/Pointer.h>
#include <m2/object/Tile.h>
#include <m2/String.h>
#include <m2/protobuf/Utils.h>
#include <m2g/Group.h>
#include <m2/Sprite.h>
#include <Level.pb.h>
#include <m2g/Object.h>
#include <m2g/Ui.h>
#include <SpriteType.pb.h>
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include <m2/sdl/Utils.hh>
#include <SDL2/SDL_image.h>
#include <m2/Editor.h>
#include "m2/Ui.h"

m2::Game* m2::Game::_instance;

void m2::Game::create_instance() {
	if (_instance) {
		throw M2FATAL("Cannot create multiple instance of Game");
	}
	_instance = new Game();
}
void m2::Game::destroy_instance() {
	delete _instance;
	_instance = nullptr;
}

m2::Game::Game() {
	update_window_dims(1600, 900); // Store default window dimensions in GAME
	if ((sdlWindow = SDL_CreateWindow("m2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowRect.w, windowRect.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == nullptr) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_SetWindowMinimumSize(sdlWindow, 712, 400);
	SDL_StopTextInput(); // Text input begins activated (sometimes)
	sdlCursor = SdlUtils_CreateCursor();
	SDL_SetCursor(sdlCursor);
	if ((pixelFormat = SDL_GetWindowPixelFormat(sdlWindow)) == SDL_PIXELFORMAT_UNKNOWN) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Unset: pixelated sprites, "1": filtered sprites
	if ((sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED)) == nullptr) { // SDL_RENDERER_PRESENTVSYNC
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	//SDL_SetTextureColorMod(sdlTexture, 127, 127, 127); Temporarily disabled, because lighting is disabled
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
	if ((ttfFont = TTF_OpenFont("resource/fonts/VT323/VT323-Regular.ttf", 32)) == nullptr) {
		throw M2FATAL("SDL error: " + std::string{TTF_GetError()});
	}

	sprite_sheets = load_sprite_sheets(std::string{m2g::sprite_sheets}, sdlRenderer);
	sprite_effects_sheet = SpriteEffectsSheet{sdlRenderer};
	foreground_companions_sheet = ForegroundCompanionsSheet{sdlRenderer};
	sprites = load_sprites(sprite_sheets, *sprite_effects_sheet, *foreground_companions_sheet);
	editor_background_sprites = list_editor_background_sprites(sprite_sheets);
	editor_object_sprites = list_editor_object_sprites(std::string{m2g::objects});
	glyphs_sheet = GlyphsSheet{sdlRenderer};
	shapes_sheet = ShapesSheet{sdlRenderer};
	items = load_items(std::string{m2g::items});
	animations = load_animations(std::string{m2g::animations});
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
	auto lb = proto::json_file_to_message<pb::Level>(level_resource_path);
	m2_reflect_failure(lb);

	if (level) {
		unload_level();
	}
	auto level_value = Level::create_single_player_level(level_resource_path);
	m2_reflect_failure(level_value);
	level = *level_value;

	// Reset state
	events.clear();
	is_phy_stepping = false;
	GAME.world = new b2World(m2g::gravity ? b2Vec2{0.0f, 10.0f} : b2Vec2{});
	GAME.contactListener = new m2::box2d::ContactListener(m2::comp::Physique::contact_cb);
	GAME.world->SetContactListener(GAME.contactListener);

	// Create background tiles
	for (int y = 0; y < lb->background_rows_size(); ++y) {
		for (int x = 0; x < lb->background_rows(y).items_size(); ++x) {
			auto sprite_type = lb->background_rows(y).items(x);
			if (sprite_type) {
				obj::create_tile(Vec2f{x, y} + Vec2f{0.5f, 0.5f}, sprites[sprite_type]);
			}
		}
	}
	// Create foreground objects
	for (const auto& fg_object : lb->objects()) {
		auto [obj, id] = m2::create_object(m2::Vec2f{fg_object.position()} + Vec2f{0.5f, 0.5f});

		// Assign to group
		if (fg_object.has_group() && fg_object.group().type() != m2g::pb::GroupType::NO_GROUP) {
			GroupId group_id{fg_object.group()};

			Group *group;
			auto group_it = groups.find(group_id);
			if (group_it != groups.end()) {
				group = group_it->second.get();
			} else {
				group = m2g::create_group(group_id.type);
				groups[group_id] = std::unique_ptr<Group>(group);
			}
			obj.set_group(group_id, group->add_member(id));
		}

		auto load_result = m2g::fg_object_loader(obj, fg_object.type());
		m2_reflect_failure(load_result);
	}
	// Init pathfinder map
	PathfinderMap_Init(&GAME.pathfinderMap);

	// Create default objects
	m2::obj::create_camera();
	m2::obj::create_pointer();

	// Init HUD
	GAME.leftHudUIState = m2::ui::State(&m2g::ui::left_hud);
	GAME.leftHudUIState->update_positions(GAME.leftHudRect);
	GAME.leftHudUIState->update_contents();
	GAME.rightHudUIState = m2::ui::State(&m2g::ui::right_hud);
	GAME.rightHudUIState->update_positions(GAME.rightHudRect);
	GAME.rightHudUIState->update_contents();

	return {};
}

m2::VoidValue m2::Game::load_editor(const std::string& level_resource_path) {
	// Reset state
	events.clear();
	is_phy_stepping = false;

	if (level) {
		unload_level();
	}

	auto level_value = Level::create_editor_level(level_resource_path);
	m2_reflect_failure(level_value);
	level = std::move(*level_value);

	// Create default objects
	playerId = m2::obj::create_god();
	m2::obj::create_camera();
	m2::obj::create_origin();

	// UI Hud
	GAME.leftHudUIState = m2::ui::State(&ui::editor_left_hud);
	GAME.leftHudUIState->update_positions(GAME.leftHudRect);
	GAME.leftHudUIState->update_contents();
	GAME.rightHudUIState = m2::ui::State(&ui::editor_right_hud);
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

void m2::Game::update_window_dims(int window_width, int window_height) {
	windowRect = SDL_Rect{0, 0, window_width, window_height};

	auto ideal_width = window_height * GAME_AND_HUD_ASPECT_RATIO_MUL / GAME_AND_HUD_ASPECT_RATIO_DIV;
	if (window_width < ideal_width) {
		// Screen is taller than expected
		int provisional_game_height = window_width * GAME_AND_HUD_ASPECT_RATIO_DIV / GAME_AND_HUD_ASPECT_RATIO_MUL;
		game_ppm = provisional_game_height * game_height_div_m / game_height_mul_m;
	} else {
		// Screen is exact or wider
		game_ppm = window_height * game_height_div_m / game_height_mul_m;
	}

	int game_height = game_ppm * game_height_mul_m / game_height_div_m;
	int game_width = game_height * GAME_ASPECT_RATIO_MUL / GAME_ASPECT_RATIO_DIV;
	int hud_height = game_height;
	int hud_width = game_height * HUD_ASPECT_RATIO_MUL / HUD_ASPECT_RATIO_DIV;

	int top_envelope_size = (window_height - game_height) / 2;
	int bottom_envelope_size = (window_height - game_height) - top_envelope_size;
	int left_envelope_size = (window_width - game_width - 2 * hud_width) / 2;
	int right_envelope_size = (window_width - game_width - 2 * hud_width) - left_envelope_size;

	topEnvelopeRect = SDL_Rect{0, 0, window_width, top_envelope_size};
	bottomEnvelopeRect = SDL_Rect{0, top_envelope_size + game_height, window_width, bottom_envelope_size};
	leftEnvelopeRect = SDL_Rect{0, top_envelope_size, left_envelope_size, game_height};
	rightEnvelopeRect = SDL_Rect{left_envelope_size + 2 * hud_width + game_width, top_envelope_size, right_envelope_size, game_height};

	gameAndHudRect = SDL_Rect{left_envelope_size, top_envelope_size, 2 * hud_width + game_width, game_height};
	leftHudRect = SDL_Rect{left_envelope_size, top_envelope_size, hud_width, hud_height};
	rightHudRect = SDL_Rect{left_envelope_size + hud_width + game_width, top_envelope_size, hud_width, hud_height};
	gameRect = SDL_Rect{left_envelope_size + hud_width, top_envelope_size, game_width, game_height};

	console_rect = SDL_Rect{gameRect.x, gameRect.y + gameRect.h * 22 / 24, gameRect.w, gameRect.h * 2 / 24};
}

void m2::Game::update_mouse_position() {
	auto& cam = objects[cameraId];
	m2::Vec2f cameraPosition = cam.position;

	m2::Vec2i pointerPosition = events.mouse_position();
	m2::Vec2i pointerPositionWRTScreenCenter_px = m2::Vec2i{pointerPosition.x - (windowRect.w / 2), pointerPosition.y - (windowRect.h / 2) };
	mousePositionWRTScreenCenter_m = m2::Vec2f{(float) pointerPositionWRTScreenCenter_px.x / game_ppm, (float) pointerPositionWRTScreenCenter_px.y / game_ppm };
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

std::pair<int, int> m2::Game::pixel_scale_mul_div(int sprite_ppm) const {
	return std::make_pair(game_ppm, sprite_ppm);
}
