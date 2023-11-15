#include "m2/Game.h"
#include <m2/Exception.h>
#include <m2/Object.h>
#include <m2/String.h>
#include <m2/Sprite.h>
#include <Level.pb.h>
#include <m2/sheet_editor/Ui.h>
#include "m2/component/Graphic.h"
#include <m2/sdl/Detail.hh>
#include <SDL2/SDL_image.h>
#include <filesystem>
#include <ranges>

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
	// Default Metal backend is slow in 2.5D mode, while drawing the rectangle debug shapes
//	if (SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl") == false) {
//		LOG_WARN("Failed to set opengl as render hint");
//	}
	// Use the driver line API
	if (SDL_SetHint(SDL_HINT_RENDER_LINE_METHOD, "2") == false) {
		LOG_WARN("Failed to set line render method");
	}

	recalculate_dimensions(800, 450, m2g::default_game_height_m);
	if ((window = SDL_CreateWindow("m2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _dims.window.w, _dims.window.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == nullptr) {
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
	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	LOG_INFO("Renderer", info.name);

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
	shapes_sheet = ShapesSheet{renderer};
	dynamic_sheet = DynamicSheet{renderer};

	// Load game resources
	const std::filesystem::path _resource_dir("resource");
	resource_dir = _resource_dir / "game" / m2g::game_name;
	levels_dir = _resource_dir / "game" / m2g::game_name / "levels";

	sprite_sheets = load_sprite_sheets(resource_dir / "SpriteSheets.json", renderer);
	_sprites = load_sprites(sprite_sheets, *sprite_effects_sheet);
	level_editor_background_sprites = list_level_editor_background_sprites(sprite_sheets);
	level_editor_object_sprites = list_level_editor_object_sprites(resource_dir / "Objects.json");
	_items = load_items(resource_dir / "Items.json");
	animations = load_animations(resource_dir / "Animations.json");
	_songs = load_songs(resource_dir / "Songs.json");
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

m2::void_expected m2::Game::load_single_player(const std::variant<std::filesystem::path,pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_single_player(level_path_or_blueprint, level_name);
}
m2::void_expected m2::Game::load_level_editor(const std::string& level_resource_path) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_level_editor(level_resource_path);
}
m2::void_expected m2::Game::load_pixel_editor(const std::string& image_resource_path, int x_offset, int y_offset) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_pixel_editor(image_resource_path, x_offset, y_offset);
}

m2::void_expected m2::Game::load_sheet_editor(const std::string& sheet_path) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_sheet_editor(sheet_path);
}

void m2::Game::reset_state() {
	events.clear();
}

const m2::Song& m2::Game::get_song(m2g::pb::SongType song_type) {
	return _songs[protobuf::enum_index(song_type)];
}

void m2::Game::handle_quit_event() {
	if (events.pop_quit()) {
		quit = true;
	}
}

void m2::Game::handle_window_resize_event() {
	auto window_resize = events.pop_window_resize();
	if (window_resize) {
		recalculate_dimensions(window_resize->x, window_resize->y);
		set_zoom(1.0f);
	}
}

void m2::Game::handle_console_event() {
	if (events.pop_key_press(Key::CONSOLE)) {
		if (ui::State::create_execute_sync(&ui::console_ui) == ui::Action::QUIT) {
			quit = true;
		}
	}
}

void m2::Game::handle_menu_event() {
	if (events.pop_key_press(Key::MENU)) {
		// Select the correct pause menu
		const auto* pause_menu = [](const Level::Type type) -> const ui::Blueprint* {
			if (type == Level::Type::SINGLE_PLAYER) {
				return m2g::ui::pause_menu();
			}
			if (type == Level::Type::SHEET_EDITOR) {
				return &ui::sheet_editor_main_menu;
			}
			return nullptr;
		}(level().type());
		// Execute pause menu if found, exit if QUIT is returned
		if (pause_menu && ui::State::create_execute_sync(pause_menu) == ui::Action::QUIT) {
			quit = true;
		}
	}
}

void m2::Game::handle_hud_events() {
	IF(_level->left_hud_ui_state)->handle_events(events);
	IF(_level->right_hud_ui_state)->handle_events(events);
	IF(_level->message_box_ui_state)->handle_events(events);
}

void m2::Game::execute_pre_step() {
	for (auto phy_it: _level->physics) {
		IF(phy_it.first->pre_step)(*phy_it.first);
	}
	execute_deferred_actions();
}

void m2::Game::update_characters() {
	for (auto char_it: _level->characters) {
		auto &chr = get_character_base(*char_it.first);
		chr.automatic_update();
	}
	for (auto char_it: _level->characters) {
		auto &chr = get_character_base(*char_it.first);
		IF(chr.update)(chr);
	}
	execute_deferred_actions();
}

void m2::Game::execute_step() {
	if (_level->world) {
		LOGF_TRACE("Stepping world %f seconds...", phy_period);
		_level->world->Step(phy_period, velocity_iterations, position_iterations);
		LOG_TRACE("World stepped");
		// Update positions
		for (auto physique_it: _level->physics) {
			auto &phy = *physique_it.first;
			if (phy.body) {
				auto &object = phy.parent();
				auto old_pos = object.position;
				// Update draw list
				object.position = m2::VecF{phy.body->GetPosition()};
				if (old_pos != object.position) {
					_level->draw_list.queue_update(phy.object_id, object.position);
				}
			}
		}
	}
	// Re-sort draw list
	_level->draw_list.update();
	if (not m2g::world_is_static) {
		// If the world is NOT static, the pathfinder's cache should be cleared, because the objects might have been moved
		_level->pathfinder->clear_cache();
	}
	execute_deferred_actions();
}

void m2::Game::execute_post_step() {
	for (auto phy_it: _level->physics) {
		IF(phy_it.first->post_step)(*phy_it.first);
	}
	execute_deferred_actions();
}

void m2::Game::update_sounds() {
	for (auto sound_emitter_it : _level->sound_emitters) {
		IF(sound_emitter_it.first->update)(*sound_emitter_it.first);
	}
	execute_deferred_actions();
}

void m2::Game::execute_pre_draw() {
	for (auto gfx_it : _level->graphics) {
		IF(gfx_it.first->pre_draw)(*gfx_it.first);
	}
}

void m2::Game::update_hud_contents() {
	IF(_level->left_hud_ui_state)->update_contents();
	IF(_level->right_hud_ui_state)->update_contents();
	IF(_level->message_box_ui_state)->update_contents();
}

void m2::Game::clear_back_buffer() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

namespace {
	inline void draw_one_background_layer(m2::Pool<m2::Graphic>& terrain_graphics) {
		for (auto gfx_it : terrain_graphics) {
			IF(gfx_it.first->on_draw)(*gfx_it.first);
			IF(gfx_it.first->on_effect)(*gfx_it.first);
		}
	}
	inline void draw_all_background_layers(m2::Level& level) {
		// Draw all background layers
		for (auto& terrain_graphics : std::ranges::reverse_view(level.terrain_graphics)) {
			draw_one_background_layer(terrain_graphics);
		}
	}
}
void m2::Game::draw_background() {
	if (_level->type() == Level::Type::LEVEL_EDITOR) {
		std::visit(m2::overloaded {
			[&](MAYBE const ledit::State::PaintMode& mode) { draw_one_background_layer(_level->terrain_graphics[I(_level->level_editor_state->selected_layer)]); },
			[&](MAYBE const ledit::State::EraseMode& mode) { draw_one_background_layer(_level->terrain_graphics[I(_level->level_editor_state->selected_layer)]); },
			[&](MAYBE const ledit::State::PickMode& mode) { draw_one_background_layer(_level->terrain_graphics[I(_level->level_editor_state->selected_layer)]); },
			[&](MAYBE const ledit::State::SelectMode& mode) { draw_one_background_layer(_level->terrain_graphics[I(_level->level_editor_state->selected_layer)]); },
			[&](MAYBE const auto& mode) { draw_all_background_layers(*_level); },
		}, _level->level_editor_state->mode);
	} else {
		draw_all_background_layers(*_level);
	}
}

void m2::Game::draw_foreground() {
	for (const auto& gfx_id : _level->draw_list) {
		auto& gfx = _level->graphics[gfx_id];
		IF(gfx.on_draw)(gfx);
		IF(gfx.on_effect)(gfx);
	}
}

void m2::Game::draw_lights() {
	for (auto light_it : _level->lights) {
		IF(light_it.first->on_draw)(*light_it.first);
	}
}

void m2::Game::execute_post_draw() {
	for (auto gfx_it : _level->graphics) {
		IF(gfx_it.first->post_draw)(*gfx_it.first);
	}
}

void m2::Game::debug_draw() {
#ifdef DEBUG
	for (auto phy_it: _level->physics) {
		IF(phy_it.first->on_debug_draw)(*phy_it.first);
	}

	if (is_projection_type_perspective(_level->projection_type())) {
		SDL_SetRenderDrawColor(GAME.renderer, 255, 255, 255, 127);
		for (int y = 0; y < 20; ++y) {
			for (int x = 0; x < 20; ++x) {
				m3::VecF p = {x, y, 0};
				auto projected_p = screen_origin_to_projection_along_camera_plane_dstpx(_level->projection_type(), p);
				if (projected_p) {
					SDL_RenderDrawPointF(GAME.renderer, projected_p->x, projected_p->y);
				}
			}
		}
	}
#endif
}

void m2::Game::draw_hud() {
	IF(_level->left_hud_ui_state)->draw();
	IF(_level->right_hud_ui_state)->draw();
	IF(_level->message_box_ui_state)->draw();
}

void m2::Game::draw_envelopes() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &_dims.top_envelope);
	SDL_RenderFillRect(renderer, &_dims.bottom_envelope);
	SDL_RenderFillRect(renderer, &_dims.left_envelope);
	SDL_RenderFillRect(renderer, &_dims.right_envelope);
}

void m2::Game::flip_buffers() {
	SDL_RenderPresent(renderer);
}

void m2::Game::recalculate_dimensions(int window_width, int window_height, int game_height_m) {
	_dims = Dimensions{game_height_m == 0 ? _dims.height_m : game_height_m, window_width, window_height};
}

void m2::Game::set_zoom(float game_height_multiplier) {
	recalculate_dimensions(_dims.window.w, _dims.window.h, iround(static_cast<float>(_dims.height_m) * game_height_multiplier));
	if (_level) {
		IF(_level->left_hud_ui_state)->update_positions(_dims.left_hud);
		IF(_level->right_hud_ui_state)->update_positions(_dims.right_hud);
		IF(_level->message_box_ui_state)->update_positions(_dims.message_box);
	}
}

void m2::Game::recalculate_mouse_position() {
	auto mouse_position = events.mouse_position();
	auto screen_center_to_mouse_position_px = VecI{mouse_position.x - (_dims.window.w / 2), mouse_position.y - (_dims.window.h / 2)};
	_screen_center_to_mouse_position_m = VecF{F(screen_center_to_mouse_position_px.x) / F(_dims.ppm), F(screen_center_to_mouse_position_px.y) / F(_dims.ppm)};

	if (is_projection_type_perspective(_level->projection_type())) {
		// Mouse moves on the plane centered at the player looking towards the camera
		// Find m3::VecF of the mouse position in the world starting from the player position
		auto sin_of_player_to_camera_angle = m2g::camera_height / m2g::camera_distance;
		auto cos_of_player_to_camera_angle = sqrtf(1.0f - sin_of_player_to_camera_angle * sin_of_player_to_camera_angle);

		auto y_offset = (F(screen_center_to_mouse_position_px.y) / m3::ppm(_level->projection_type())) * sin_of_player_to_camera_angle;
		auto z_offset = -(F(screen_center_to_mouse_position_px.y) / m3::ppm(_level->projection_type())) * cos_of_player_to_camera_angle;
		auto x_offset = F(screen_center_to_mouse_position_px.x) / m3::ppm(_level->projection_type());
		auto player_position = m3::focus_position_m();
		auto mouse_position_world_m = m3::VecF{player_position.x + x_offset, player_position.y + y_offset, player_position.z + z_offset};

		// Create Line from camera to mouse position
		auto ray_to_mouse = m3::Line::from_points(m3::camera_position_m(_level->projection_type()), mouse_position_world_m);
		// Get the xy-plane
		auto plane = m3::Plane::xy_plane(m2g::xy_plane_z_component);
		// Get the intersection
		auto [intersection_point, forward_intersection] = plane.intersection(ray_to_mouse);

		if (forward_intersection) {
			_mouse_position_world_m = VecF{intersection_point.x, intersection_point.y};
		} else {
			_mouse_position_world_m = VecF{-intersection_point.x, -10000.0f}; // Infinity is 10KM
		}
	} else {
		auto camera_position = _level->objects[_level->camera_id].position;
		_mouse_position_world_m = _screen_center_to_mouse_position_m + camera_position;
	}
}

m2::VecF m2::Game::pixel_to_2d_world_m(const VecI& pixel_position) {
	auto screen_center_to_pixel_position_px = VecI{pixel_position.x - (_dims.window.w / 2), pixel_position.y - (_dims.window.h / 2)};
	auto screen_center_to_pixel_position_m = VecF{F(screen_center_to_pixel_position_px.x) / F(_dims.ppm), F(screen_center_to_pixel_position_px.y) / F(_dims.ppm)};
	auto camera_position = _level->objects[_level->camera_id].position;
	return screen_center_to_pixel_position_m + camera_position;
}

m2::RectF m2::Game::viewport_to_2d_world_rect_m() {
	auto top_left = pixel_to_2d_world_m(VecI{dimensions().game.x, dimensions().game.y});
	auto bottom_right = pixel_to_2d_world_m(VecI{dimensions().game.x + dimensions().game.w, dimensions().game.y + dimensions().game.h});
	return RectF::from_corners(top_left, bottom_right);
}

void m2::Game::recalculate_directional_audio() {
	if (_level->left_listener || _level->right_listener) {
		// Loop over sounds
		for (auto sound_emitter_it : _level->sound_emitters) {
			const auto& sound_emitter = *sound_emitter_it.first;
			const auto& sound_position = sound_emitter.parent().position;
			// Loop over playbacks
			for (auto playback_id : sound_emitter.playbacks) {
				if (!audio_manager->has_playback(playback_id)) {
					continue; // Playback may have finished (if it's ONCE)
				}
				// Left listener
				auto left_volume = _level->left_listener ? _level->left_listener->volume_of(sound_position) : 0.0f;
				audio_manager->set_playback_left_volume(playback_id, left_volume);
				// Right listener
				auto right_volume = _level->right_listener ? _level->right_listener->volume_of(sound_position) : 0.0f;
				audio_manager->set_playback_right_volume(playback_id, right_volume);
			}
		}
	}
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
