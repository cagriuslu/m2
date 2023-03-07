#include "m2/Game.h"
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
#include <m2/LevelEditor.h>
#include "m2/Ui.h"

m2::Game* m2::Game::_instance;

void m2::Game::create_instance() {
	DEBUG_FN();
	if (_instance) {
		throw M2FATAL("Cannot create multiple instance of Game");
	}
	_instance = new Game();
}
void m2::Game::destroy_instance() {
	DEBUG_FN();
	delete _instance;
	_instance = nullptr;
}

m2::Game::Game() {
	DEBUG_FN();
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

	audio_manager.emplace();

	sprite_sheets = load_sprite_sheets(std::string{m2g::sprite_sheets}, sdlRenderer);
	sprite_effects_sheet = SpriteEffectsSheet{sdlRenderer};
	sprites = load_sprites(sprite_sheets, *sprite_effects_sheet);
	level_editor_background_sprites = list_level_editor_background_sprites(sprite_sheets);
	level_editor_object_sprites = list_level_editor_object_sprites(std::string{m2g::objects});
	glyphs_sheet = GlyphsSheet{sdlRenderer};
	shapes_sheet = ShapesSheet{sdlRenderer};
	dynamic_sheet = DynamicSheet{sdlRenderer};
	_items = load_items(std::string{m2g::items});
	animations = load_animations(std::string{m2g::animations});
}

m2::Game::~Game() {
	_level.reset();
	SDL_DestroyRenderer(sdlRenderer);
	SDL_FreeCursor(sdlCursor);
	SDL_DestroyWindow(sdlWindow);
}

m2::VoidValue m2::Game::load_single_player(const std::variant<FilePath,pb::Level>& level_path_or_blueprint) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_single_player(level_path_or_blueprint);
}
m2::VoidValue m2::Game::load_level_editor(const std::string& level_resource_path) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_level_editor(level_resource_path);
}

void m2::Game::reset_state() {
	events.clear();
}

const m2::Item& m2::Game::get_item(m2g::pb::ItemType item_type) {
	static const auto* const item_type_desc = m2g::pb::ItemType_descriptor();
	return _items[item_type_desc->FindValueByNumber(item_type)->index()];
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
	auto& cam = _level->objects[_level->cameraId];
	m2::Vec2f cameraPosition = cam.position;

	m2::Vec2i pointerPosition = events.mouse_position();
	m2::Vec2i pointerPositionWRTScreenCenter_px = m2::Vec2i{pointerPosition.x - (windowRect.w / 2), pointerPosition.y - (windowRect.h / 2) };
	mousePositionWRTScreenCenter_m = m2::Vec2f{(float) pointerPositionWRTScreenCenter_px.x / (float) game_ppm, (float) pointerPositionWRTScreenCenter_px.y / (float) game_ppm };
	mousePositionWRTGameWorld_m = mousePositionWRTScreenCenter_m + cameraPosition;
}

void m2::Game::add_deferred_action(const std::function<void(void)>& action) {
	_level->deferred_actions.push_back(action);
}

void m2::Game::execute_deferred_actions() {
	for (auto& action : _level->deferred_actions) {
		action();
	}
	_level->deferred_actions.clear();
}

std::pair<int, int> m2::Game::pixel_scale_mul_div(int sprite_ppm) const {
	return std::make_pair(game_ppm, sprite_ppm);
}
