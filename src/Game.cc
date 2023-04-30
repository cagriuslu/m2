#include "m2/Game.h"
#include <m2/Glyph.h>
#include <m2/Exception.h>
#include <m2/Object.h>
#include <m2/String.h>
#include <m2/Sprite.h>
#include <Level.pb.h>
#include "m2/component/Physique.h"
#include "m2/component/Graphic.h"
#include <m2/sdl/Detail.hh>
#include <SDL2/SDL_image.h>
#include "m2/Ui.h"
#include <filesystem>

m2::Game* m2::Game::_instance;

void m2::Game::create_instance() {
	DEBUG_FN();
	if (_instance) {
		throw M2FATAL("Cannot create multiple instance of Game");
	}
	_instance = new Game();
	_instance->initialize_context();
}
void m2::Game::destroy_instance() {
	DEBUG_FN();
	delete _instance;
	_instance = nullptr;
}

m2::Game::Game() {
	DEBUG_FN();
	update_window_dims(1600, 900); // Store default window dimensions in GAME
	if ((window = SDL_CreateWindow("m2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_rect.w, window_rect.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == nullptr) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_SetWindowMinimumSize(window, 712, 400);
	SDL_StopTextInput(); // Text input begins activated (sometimes)
	cursor = SdlUtils_CreateCursor();
	SDL_SetCursor(cursor);
	if ((pixel_format = SDL_GetWindowPixelFormat(window)) == SDL_PIXELFORMAT_UNKNOWN) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Unset: pixelated sprites, "1": filtered sprites
	if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) == nullptr) { // TODO: SDL_RENDERER_PRESENTVSYNC
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	//SDL_SetTextureColorMod(sdlTexture, 127, 127, 127); Temporarily disabled, because lighting is disabled
	SDL_Surface* lightSurface = IMG_Load("resource/RadialGradient-WhiteBlack.png");
	if (lightSurface == nullptr) {
		throw M2FATAL("SDL error: " + std::string{IMG_GetError()});
	}
	if ((light_texture = SDL_CreateTextureFromSurface(renderer, lightSurface)) == nullptr) {
		throw M2FATAL("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_FreeSurface(lightSurface);
	SDL_SetTextureBlendMode(light_texture, SDL_BLENDMODE_MUL);
	SDL_SetTextureAlphaMod(light_texture, 0);
	SDL_SetTextureColorMod(light_texture, 127, 127, 127);
	if ((font = TTF_OpenFont("resource/fonts/VT323/VT323-Regular.ttf", 280)) == nullptr) {
		throw M2FATAL("SDL error: " + std::string{TTF_GetError()});
	}

	audio_manager.emplace();
	sprite_effects_sheet = SpriteEffectsSheet{renderer};
	glyphs_sheet = GlyphsSheet{renderer};
	shapes_sheet = ShapesSheet{renderer};
	dynamic_sheet = DynamicSheet{renderer};

	// Load game resources
	std::filesystem::path resource_dir("resource");
	game_resource_dir = resource_dir / "game" / m2g::game_name;

	sprite_sheets = load_sprite_sheets(game_resource_dir / "SpriteSheets.json", renderer);
	_sprites = load_sprites(sprite_sheets, *sprite_effects_sheet);
	level_editor_background_sprites = list_level_editor_background_sprites(sprite_sheets);
	level_editor_object_sprites = list_level_editor_object_sprites(game_resource_dir / "Objects.json");
	_items = load_items(game_resource_dir / "Items.json");
	animations = load_animations(game_resource_dir / "Animations.json");
	_songs = load_songs(game_resource_dir / "Songs.json");
}

m2::Game::~Game() {
	_level.reset();
	if (context) {
		m2g::destroy_context(context);
		context = nullptr;
	}
	audio_manager.reset();
	SDL_DestroyRenderer(renderer);
	SDL_FreeCursor(cursor);
	SDL_DestroyWindow(window);
}

void m2::Game::initialize_context() {
	// User might access GAME from the following function
	// We have to call it after GAME is fully constructed
	context = m2g::create_context();
}

m2::VoidValue m2::Game::load_single_player(const std::variant<FilePath,pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_single_player(level_path_or_blueprint, level_name);
}
m2::VoidValue m2::Game::load_level_editor(const std::string& level_resource_path) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_level_editor(level_resource_path);
}
m2::VoidValue m2::Game::load_pixel_editor(const std::string& image_resource_path, int x_offset, int y_offset) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_pixel_editor(image_resource_path, x_offset, y_offset);
}

void m2::Game::reset_state() {
	events.clear();
}

const m2::Song& m2::Game::get_song(m2g::pb::SongType song_type) {
	return _songs[protobuf::enum_index(song_type)];
}

void m2::Game::update_window_dims(int window_width, int window_height) {
	window_rect = SDL_Rect{0, 0, window_width, window_height};

	auto ideal_width = window_height * GAME_AND_HUD_ASPECT_RATIO_MUL / GAME_AND_HUD_ASPECT_RATIO_DIV;
	if (window_width < ideal_width) {
		// Screen is taller than expected
		auto provisional_game_height = window_width * GAME_AND_HUD_ASPECT_RATIO_DIV / GAME_AND_HUD_ASPECT_RATIO_MUL;
		_game_ppm = provisional_game_height * game_height_m.d() / game_height_m.n();
	} else {
		// Screen is exact or wider
		_game_ppm = window_height * game_height_m.d() / game_height_m.n();
	}

	int game_height = _game_ppm * game_height_m.n() / game_height_m.d();
	int game_width = game_height * GAME_ASPECT_RATIO_MUL / GAME_ASPECT_RATIO_DIV;
	int hud_height = game_height;
	int hud_width = game_height * HUD_ASPECT_RATIO_MUL / HUD_ASPECT_RATIO_DIV;

	int top_envelope_size = (window_height - game_height) / 2;
	int bottom_envelope_size = (window_height - game_height) - top_envelope_size;
	int left_envelope_size = (window_width - game_width - 2 * hud_width) / 2;
	int right_envelope_size = (window_width - game_width - 2 * hud_width) - left_envelope_size;

	top_envelope_rect = SDL_Rect{0, 0, window_width, top_envelope_size};
	bottom_envelope_rect = SDL_Rect{0, top_envelope_size + game_height, window_width, bottom_envelope_size};
	left_envelope_rect = SDL_Rect{0, top_envelope_size, left_envelope_size, game_height};
	right_envelope_rect = SDL_Rect{left_envelope_size + 2 * hud_width + game_width, top_envelope_size, right_envelope_size, game_height};

	game_and_hud_rect = SDL_Rect{left_envelope_size, top_envelope_size, 2 * hud_width + game_width, game_height};
	left_hud_rect = SDL_Rect{left_envelope_size, top_envelope_size, hud_width, hud_height};
	right_hud_rect = SDL_Rect{left_envelope_size + hud_width + game_width, top_envelope_size, hud_width, hud_height};
	game_rect = SDL_Rect{left_envelope_size + hud_width, top_envelope_size, game_width, game_height};

	console_rect = SDL_Rect{game_rect.x, game_rect.y + game_rect.h * 22 / 24, game_rect.w, game_rect.h * 2 / 24};
}

void m2::Game::update_mouse_position() {
	auto mouse_position = events.mouse_position();
	auto screen_center_to_mouse_position_px = Vec2i{mouse_position.x - (window_rect.w / 2), mouse_position.y - (window_rect.h / 2)};
	_screen_center_to_mouse_position_m = Vec2f{(float) screen_center_to_mouse_position_px.x / (float) _game_ppm, (float) screen_center_to_mouse_position_px.y / (float) _game_ppm};
	auto camera_position = _level->objects[_level->camera_id].position;
	_mouse_position_world_m = _screen_center_to_mouse_position_m + camera_position;
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
	return std::make_pair(_game_ppm, sprite_ppm);
}
