#include <m2/Game.h>
#include <m2/Log.h>

#include <Level.pb.h>
#include <SDL2/SDL_image.h>
#include <m2/Error.h>
#include <m2/Object.h>
#include <m2/Sprite.h>
#include <m2/String.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/level_editor/Ui.h>
#include <m2/sdl/Detail.h>
#include <m2/sheet_editor/Ui.h>

#include <filesystem>
#include <ranges>

#include "m2/component/Graphic.h"
#include <m2/ui/Action.h>

m2::Game* m2::Game::_instance;

void m2::Game::create_instance() {
	LOG_DEBUG("Creating Game instance...");
	if (_instance) {
		throw M2_ERROR("Cannot create multiple instance of Game");
	}
	_instance = new Game();
	LOG_DEBUG("Game instance created");

	// User might access GAME from the following function
	// We have to call it after GAME is fully constructed
	_instance->_proxy.load_resources();
}

void m2::Game::destroy_instance() {
	DEBUG_FN();
	delete _instance;
	_instance = nullptr;
}

m2::Game::Game() {
	// Default Metal backend is slow in 2.5D mode, while drawing the rectangle debug shapes
	//	if (SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl") == false) {
	//		LOG_WARN("Failed to set opengl as render hint");
	//	}
	// Use the driver line API
	if (SDL_SetHint(SDL_HINT_RENDER_LINE_METHOD, "2") == false) {
		LOG_WARN("Failed to set line render method");
	}

	recalculate_dimensions(800, 450, _proxy.default_game_height_m);
	if ((window = SDL_CreateWindow(
		"m2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _dims.window.w, _dims.window.h,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == nullptr) {
		throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_SetWindowMinimumSize(window, 712, 400);
	SDL_StopTextInput();  // Text input begins activated (sometimes)

	cursor = SdlUtils_CreateCursor();
	SDL_SetCursor(cursor);
	if ((pixel_format = SDL_GetWindowPixelFormat(window)) == SDL_PIXELFORMAT_UNKNOWN) {
		throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
	}

	// SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Unset: pixelated sprites, "1": filtered sprites
	if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) ==
		nullptr) {  // TODO: SDL_RENDERER_PRESENTVSYNC
		throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	LOG_INFO("Renderer", info.name);

	SDL_Surface* lightSurface = IMG_Load("resource/RadialGradient-WhiteBlack.png");
	if (lightSurface == nullptr) {
		throw M2_ERROR("SDL error: " + std::string{IMG_GetError()});
	}
	if ((light_texture = SDL_CreateTextureFromSurface(renderer, lightSurface)) == nullptr) {
		throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_FreeSurface(lightSurface);
	SDL_SetTextureBlendMode(light_texture, SDL_BLENDMODE_MUL);
	SDL_SetTextureAlphaMod(light_texture, 0);
	SDL_SetTextureColorMod(light_texture, 127, 127, 127);
	if ((font = TTF_OpenFont("resource/fonts/VT323/VT323-Regular.ttf", 280)) == nullptr) {
		throw M2_ERROR("SDL error: " + std::string{TTF_GetError()});
	}

	audio_manager.emplace();
	sprite_effects_sheet = SpriteEffectsSheet{renderer};
	shapes_sheet = ShapesSheet{renderer};
	dynamic_sheet = DynamicSheet{renderer};

	// Load game resources
	const std::filesystem::path _resource_dir("resource");
	resource_dir = _resource_dir / "game" / _proxy.game_name;
	levels_dir = _resource_dir / "game" / _proxy.game_name / "levels";

	auto sheets_pb = pb::json_file_to_message<pb::SpriteSheets>(resource_dir / "SpriteSheets.json");
	if (!sheets_pb) {
		throw M2_ERROR(sheets_pb.error());
	}
	sprite_sheets = load_sprite_sheets(*sheets_pb, renderer, _proxy.lightning);
	_sprites =
		load_sprites(sprite_sheets, sheets_pb->text_labels(), *sprite_effects_sheet, font, renderer, _proxy.lightning);
	level_editor_background_sprites = list_level_editor_background_sprites(_sprites);
	object_main_sprites = list_level_editor_object_sprites(resource_dir / "Objects.json");
	named_items = pb::LUT<m2::pb::Item, NamedItem>::load(resource_dir / "Items.json", &m2::pb::Items::items);
	animations =
		pb::LUT<m2::pb::Animation, Animation>::load(resource_dir / "Animations.json", &m2::pb::Animations::animations);
	songs = pb::LUT<m2::pb::Song, Song>::load(resource_dir / "Songs.json", &m2::pb::Songs::songs);
}

m2::Game::~Game() {
	_level.reset();
	audio_manager.reset();
	SDL_DestroyRenderer(renderer);
	SDL_FreeCursor(cursor);
	SDL_DestroyWindow(window);
}

m2::void_expected m2::Game::host_game(mplayer::Type type, unsigned max_connection_count) {
	LOG_INFO("Creating server instance...");
	_server_thread.emplace(type, max_connection_count);
	// Wait until the server is up
	while (not _server_thread->is_listening()) {
		SDL_Delay(25);
	}

	LOG_INFO("Server is listening, joining the game as client...");
	join_game(type, "127.0.0.1");
	// Wait until the client is connected
	while (not _client_thread->is_connected()) {
		SDL_Delay(25);
	}
	LOG_INFO("Client connected, becoming ready...");

	_client_thread->set_ready_blocking(true);
	LOG_INFO("Became ready");

	return {};
}

m2::void_expected m2::Game::join_game(mplayer::Type type, const std::string& addr) {
	_client_thread.emplace(type, addr);
	return {};
}

m2::void_expected m2::Game::load_single_player(
	const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_single_player(level_path_or_blueprint, level_name);
}

m2::void_expected m2::Game::load_multi_player_as_host(
	const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_multi_player_as_host(level_path_or_blueprint, level_name);
}

m2::void_expected m2::Game::load_multi_player_as_guest(
	const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_multi_player_as_guest(level_path_or_blueprint, level_name);
}

m2::void_expected m2::Game::load_level_editor(const std::string& level_resource_path) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_level_editor(level_resource_path);
}

m2::void_expected m2::Game::load_pixel_editor(
	const std::string& image_resource_path, const int x_offset, const int y_offset) {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_pixel_editor(image_resource_path, x_offset, y_offset);
}

m2::void_expected m2::Game::load_sheet_editor() {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_sheet_editor(resource_dir / "SpriteSheets.json");
}

m2::void_expected m2::Game::load_bulk_sheet_editor() {
	_level.reset();
	reset_state();
	_level.emplace();
	return _level->init_bulk_sheet_editor(resource_dir / "SpriteSheets.json");
}

void m2::Game::reset_state() { events.clear(); }

void m2::Game::handle_quit_event() {
	if (events.pop_quit()) {
		quit = true;
	}
}

void m2::Game::handle_window_resize_event() {
	if (const auto window_resize = events.pop_window_resize(); window_resize) {
		recalculate_dimensions(window_resize->x, window_resize->y);
		set_zoom(1.0f);
	}
}

void m2::Game::handle_console_event() {
	if (events.pop_key_press(Key::CONSOLE)) {
		if (ui::State::create_execute_sync(&ui::console_ui).is_quit()) {
			quit = true;
		}
	}
}

void m2::Game::handle_menu_event() {
	if (events.pop_key_press(Key::MENU)) {
		// Select the correct pause menu
		const ui::Blueprint* pause_menu{};
		if (std::holds_alternative<splayer::State>(level().type_state)) {
			pause_menu = _proxy.pause_menu();
		} else if (std::holds_alternative<ledit::State>(level().type_state)) {
			pause_menu = &level_editor::ui::menu;
		} else if (std::holds_alternative<sedit::State>(level().type_state)) {
			pause_menu = &ui::sheet_editor_main_menu;
		} else if (std::holds_alternative<bsedit::State>(level().type_state)) {
			pause_menu = &ui::bulk_sheet_editor_pause_menu;
		}

		// Execute pause menu if found, exit if QUIT is returned
		if (pause_menu && ui::State::create_execute_sync(pause_menu).is_quit()) {
			quit = true;
		}
	}
}

void m2::Game::handle_hud_events() {
	if (_level->custom_ui_dialog_state.second) {
		_level->custom_ui_dialog_state.second->handle_events(events)
			.if_any_return([this]() {
				// If the dialog returned, remove the state
				_level->custom_ui_dialog_state.second.reset();
			});
		// TODO handle quit
		// If there's a UI dialog, no events will be delivered to rest of the UI states and the game world
		events.clear();
		// handle_events of others are executed regardless, because there may be timing related actions
	}

	// The order of event handling is the reverse of the drawing order because custom_ui_states are assumed to be in
	// front the HUDs.
	for (auto& custom_ui : _level->custom_ui_state) { // TODO iterate backwards
		if (custom_ui.second) {
			custom_ui.second->handle_events(events)
			.if_any_return([&custom_ui]() {
				// If UI returned, remove the state
				custom_ui.second.reset();
			});
			// TODO handle quit
		}
	}
	IF(_level->message_box_ui_state)->handle_events(events);  // For disable_after
	IF(_level->left_hud_ui_state)->handle_events(events);
	IF(_level->right_hud_ui_state)->handle_events(events);
}

void m2::Game::execute_pre_step() {
	for (auto& phy : _level->physics) {
		IF(phy.pre_step)(phy);
	}
	if (_server_thread) {
		auto client_command = _server_thread->pop_turn_holder_command();
		if (client_command) {
			auto new_turn_holder =
				_proxy.handle_client_command(_server_thread->turn_holder_index(), client_command->client_command()); ////////
			if (new_turn_holder) {
				if (*new_turn_holder < 0) {
					_server_thread->shutdown();
				} else {
					_server_thread->set_turn_holder(*new_turn_holder);
					_server_update_necessary = true;
				}
			}
		}
	}
	if (_client_thread) {
		// Handle ServerCommand
		if (auto server_command = M2_GAME.client_thread().pop_server_command()) {
			_proxy.handle_server_command(*server_command);
		}
	}
}

void m2::Game::update_characters() {
	for (auto& character : _level->characters) {
		auto& chr = to_character_base(character);
		chr.automatic_update();
	}
	for (auto& character : _level->characters) {
		auto& chr = to_character_base(character);
		IF(chr.update)(chr);
	}
}

void m2::Game::execute_step() {
	if (_level->world) {
		LOGF_TRACE("Stepping world %f seconds...", phy_period);
		_level->world->Step(phy_period, velocity_iterations, position_iterations);
		LOG_TRACE("World stepped");
		// Update positions
		for (auto& phy : _level->physics) {
			if (phy.body) {
				auto& object = phy.parent();
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
	if (not _proxy.world_is_static) {
		// If the world is NOT static, the pathfinder's cache should be cleared, because the objects might have been
		// moved
		_level->pathfinder->clear_cache();
	}
}

void m2::Game::execute_post_step() {
	if (_server_thread) {
		if (_server_update_necessary) {
			// Publish ServerUpdate
			_server_thread->send_server_update();
			_server_update_necessary = false;
		}
	} else if (_client_thread) {
		// Handle ServerUpdate
		auto expect_server_update = M2_GAME.client_thread().process_server_update();
		m2_succeed_or_throw_error(expect_server_update);
		_proxy.post_server_update(*M2_GAME.client_thread().last_processed_server_update());
	}
	for (auto& phy : _level->physics) {
		IF(phy.post_step)(phy);
	}
}

void m2::Game::update_sounds() {
	for (auto& sound_emitter : _level->sound_emitters) {
		IF(sound_emitter.update)(sound_emitter);
	}
}

void m2::Game::execute_pre_draw() {
	for (auto& gfx : _level->graphics) {
		if (gfx.enabled) {
			IF(gfx.pre_draw)(gfx);
		}
	}
}

void m2::Game::update_hud_contents() {
	IF(_level->left_hud_ui_state)->update_contents();
	IF(_level->right_hud_ui_state)->update_contents();
	IF(_level->message_box_ui_state)->update_contents();
	for (auto& custom_ui : _level->custom_ui_state) {
		IF(custom_ui.second)->update_contents();
	}
	IF(_level->custom_ui_dialog_state.second)->update_contents();
}

void m2::Game::clear_back_buffer() const {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

namespace {
	void draw_one_background_layer(m2::Pool<m2::Graphic>& terrain_graphics) {
		for (auto& gfx : terrain_graphics) {
			if (gfx.enabled && gfx.draw) {
				IF(gfx.on_draw)(gfx);
				IF(gfx.on_addon)(gfx);
			}
		}
	}

	void draw_all_background_layers(m2::Level& level) {
		// Draw all background layers
		for (auto& terrain_graphics : std::ranges::reverse_view(level.terrain_graphics)) {
			draw_one_background_layer(terrain_graphics);
		}
	}
}  // namespace
void m2::Game::draw_background() {
	if (std::holds_alternative<ledit::State>(_level->type_state)) {
		const auto& le = std::get<ledit::State>(_level->type_state);
		std::visit(
			overloaded{
				[&](MAYBE const ledit::State::PaintMode& mode) {
					draw_one_background_layer(_level->terrain_graphics[I(le.selected_layer)]);
				},
				[&](MAYBE const ledit::State::EraseMode& mode) {
					draw_one_background_layer(_level->terrain_graphics[I(le.selected_layer)]);
				},
				[&](MAYBE const ledit::State::PickMode& mode) {
					draw_one_background_layer(_level->terrain_graphics[I(le.selected_layer)]);
				},
				[&](MAYBE const ledit::State::SelectMode& mode) {
					draw_one_background_layer(_level->terrain_graphics[I(le.selected_layer)]);
				},
				[&](MAYBE const auto& mode) { draw_all_background_layers(*_level); },
			},
			le.mode);
	} else {
		draw_all_background_layers(*_level);
	}
}

void m2::Game::draw_foreground() {
	for (const auto& gfx_id : _level->draw_list) {
		auto& gfx = _level->graphics[gfx_id];
		if (gfx.enabled && gfx.draw) {
			IF(gfx.on_draw)(gfx);
			IF(gfx.on_addon)(gfx);
		}
	}
}

void m2::Game::draw_lights() {
	for (auto& light : _level->lights) {
		IF(light.on_draw)(light);
	}
}

void m2::Game::execute_post_draw() {
	for (auto& gfx : _level->graphics) {
		if (gfx.enabled) {
			IF(gfx.post_draw)(gfx);
		}
	}
}

void m2::Game::debug_draw() {
#ifdef DEBUG
	for (auto& phy : _level->physics) {
		IF(phy.on_debug_draw)(phy);
	}

	if (is_projection_type_perspective(_level->projection_type())) {
		SDL_SetRenderDrawColor(M2_GAME.renderer, 255, 255, 255, 127);
		for (int y = 0; y < 20; ++y) {
			for (int x = 0; x < 20; ++x) {
				m3::VecF p = {x, y, 0};
				if (const auto projected_p = screen_origin_to_projection_along_camera_plane_dstpx(p); projected_p) {
					SDL_RenderDrawPointF(M2_GAME.renderer, projected_p->x, projected_p->y);
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
	for (auto& custom_ui : _level->custom_ui_state) {
		IF(custom_ui.second)->draw();
	}
	IF(_level->custom_ui_dialog_state.second)->draw();
}

void m2::Game::draw_envelopes() const {
	SDL_Rect sdl_rect{};

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	sdl_rect = static_cast<SDL_Rect>(_dims.top_envelope);
	SDL_RenderFillRect(renderer, &sdl_rect);
	sdl_rect = static_cast<SDL_Rect>(_dims.bottom_envelope);
	SDL_RenderFillRect(renderer, &sdl_rect);
	sdl_rect = static_cast<SDL_Rect>(_dims.left_envelope);
	SDL_RenderFillRect(renderer, &sdl_rect);
	sdl_rect = static_cast<SDL_Rect>(_dims.right_envelope);
	SDL_RenderFillRect(renderer, &sdl_rect);
}

void m2::Game::flip_buffers() const { SDL_RenderPresent(renderer); }

void m2::Game::recalculate_dimensions(const int window_width, const int window_height, const int game_height_m) {
	_dims = Dimensions{game_height_m == 0 ? _dims.height_m : game_height_m, window_width, window_height};
}

void m2::Game::set_zoom(const float game_height_multiplier) {
	recalculate_dimensions(
		_dims.window.w, _dims.window.h, iround(static_cast<float>(_dims.height_m) * game_height_multiplier));
	if (_level) {
		IF(_level->left_hud_ui_state)->update_positions(_dims.left_hud);
		IF(_level->right_hud_ui_state)->update_positions(_dims.right_hud);
		IF(_level->message_box_ui_state)->update_positions(_dims.message_box);
		for (auto& custom_ui : _level->custom_ui_state) {
			IF (custom_ui.second)->update_positions(_dims.game_and_hud.ratio(custom_ui.first));
		}
		IF (_level->custom_ui_dialog_state.second)->update_positions(_dims.game_and_hud.ratio(_level->custom_ui_dialog_state.first));
	}
}

void m2::Game::for_each_sprite(const std::function<bool(m2g::pb::SpriteType, const Sprite&)>& op) const {
	for (int i = 0; i < pb::enum_value_count<m2g::pb::SpriteType>(); ++i) {
		auto type = pb::enum_value<m2g::pb::SpriteType>(i);
		if (!op(type, get_sprite(type))) {
			return;
		}
	}
}

void m2::Game::for_each_named_item(const std::function<bool(m2g::pb::ItemType, const NamedItem&)>& op) const {
	for (int i = 0; i < pb::enum_value_count<m2g::pb::ItemType>(); ++i) {
		auto type = pb::enum_value<m2g::pb::ItemType>(i);
		if (!op(type, get_named_item(type))) {
			return;
		}
	}
}

const m2::VecF& m2::Game::mouse_position_world_m() const {
	if (not _mouse_position_world_m) {
		recalculate_mouse_position2();
	}
	return *_mouse_position_world_m;
}

const m2::VecF& m2::Game::screen_center_to_mouse_position_m() const {
	if (not _screen_center_to_mouse_position_m) {
		recalculate_mouse_position2();
	}
	return *_screen_center_to_mouse_position_m;
}

m2::VecF m2::Game::pixel_to_2d_world_m(const VecI& pixel_position) {
	const auto screen_center_to_pixel_position_px =
		VecI{pixel_position.x - (_dims.window.w / 2), pixel_position.y - (_dims.window.h / 2)};
	const auto screen_center_to_pixel_position_m = VecF{
		F(screen_center_to_pixel_position_px.x) / F(_dims.ppm), F(screen_center_to_pixel_position_px.y) / F(_dims.ppm)};
	const auto camera_position = _level->objects[_level->camera_id].position;
	return screen_center_to_pixel_position_m + camera_position;
}

m2::RectF m2::Game::viewport_to_2d_world_rect_m() {
	const auto top_left = pixel_to_2d_world_m(VecI{dimensions().game.x, dimensions().game.y});
	const auto bottom_right =
		pixel_to_2d_world_m(VecI{dimensions().game.x + dimensions().game.w, dimensions().game.y + dimensions().game.h});
	return RectF::from_corners(top_left, bottom_right);
}

void m2::Game::recalculate_directional_audio() {
	if (_level->left_listener || _level->right_listener) {
		// Loop over sounds
		for (auto& sound_emitter : _level->sound_emitters) {
			const auto& sound_position = sound_emitter.parent().position;
			// Loop over playbacks
			for (const auto playback_id : sound_emitter.playbacks) {
				if (!audio_manager->has_playback(playback_id)) {
					continue;  // Playback may have finished (if it's ONCE)
				}
				// Left listener
				const auto left_volume =
					_level->left_listener ? _level->left_listener->volume_of(sound_position) : 0.0f;
				audio_manager->set_playback_left_volume(playback_id, left_volume);
				// Right listener
				const auto right_volume =
					_level->right_listener ? _level->right_listener->volume_of(sound_position) : 0.0f;
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

void m2::Game::recalculate_mouse_position2() const {
	const auto mouse_position = events.mouse_position();
	const auto screen_center_to_mouse_position_px =
		VecI{mouse_position.x - (_dims.window.w / 2), mouse_position.y - (_dims.window.h / 2)};
	_screen_center_to_mouse_position_m = VecF{
		F(screen_center_to_mouse_position_px.x) / F(_dims.ppm), F(screen_center_to_mouse_position_px.y) / F(_dims.ppm)};

	if (is_projection_type_perspective(_level->projection_type())) {
		// Mouse moves on the plane centered at the player looking towards the camera
		// Find m3::VecF of the mouse position in the world starting from the player position
		const auto sin_of_player_to_camera_angle = M2_LEVEL.camera_offset().z / M2_LEVEL.camera_offset().length();
		const auto cos_of_player_to_camera_angle =
			sqrtf(1.0f - sin_of_player_to_camera_angle * sin_of_player_to_camera_angle);

		const auto y_offset = (F(screen_center_to_mouse_position_px.y) / m3::ppm()) * sin_of_player_to_camera_angle;
		const auto z_offset = -(F(screen_center_to_mouse_position_px.y) / m3::ppm()) * cos_of_player_to_camera_angle;
		const auto x_offset = F(screen_center_to_mouse_position_px.x) / m3::ppm();
		const auto player_position = m3::focus_position_m();
		const auto mouse_position_world_m =
			m3::VecF{player_position.x + x_offset, player_position.y + y_offset, player_position.z + z_offset};

		// Create Line from camera to mouse position
		const auto ray_to_mouse = m3::Line::from_points(m3::camera_position_m(), mouse_position_world_m);
		// Get the xy-plane
		const auto plane = m3::Plane::xy_plane(_proxy.xy_plane_z_component);
		// Get the intersection
		if (const auto [intersection_point, forward_intersection] = plane.intersection(ray_to_mouse);
			forward_intersection) {
			_mouse_position_world_m = VecF{intersection_point.x, intersection_point.y};
		} else {
			_mouse_position_world_m = VecF{-intersection_point.x, -10000.0f};  // Infinity is 10KM
		}
	} else {
		const auto camera_position = _level->objects[_level->camera_id].position;
		_mouse_position_world_m = *_screen_center_to_mouse_position_m + camera_position;
	}
}
