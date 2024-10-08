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
#include <m2/FileSystem.h>
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
	if (SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl") == false) {
		LOG_WARN("Failed to set opengl as render hint");
	}
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

	SDL_Surface* lightSurface = IMG_Load((resource_path() / "RadialGradient-WhiteBlack.png").c_str());
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
	// Open font
	if ((font = TTF_OpenFont((resource_path() / _proxy.default_font_path).c_str(), _proxy.default_font_size)) == nullptr) {
		throw M2_ERROR("SDL error: " + std::string{TTF_GetError()});
	}
	// Check font properties
	if (not TTF_FontFaceIsFixedWidth(font)) {
		// Many calculations related to text assumes a monospaced font
		throw M2_ERROR("Font is not monospaced");
	}
	int font_letter_width, font_letter_height;
	if (TTF_SizeUTF8(font, "A", &font_letter_width, &font_letter_height)) {
		throw M2_ERROR("Unable to measure the font letter size");
	}
	LOG_INFO("Font letter size", font_letter_width, font_letter_height); // 34,16
	_font_letter_width_to_height_ratio = Rational{font_letter_width, font_letter_height};
	// Font size, is the size of the letter from the baseline (ascent).
	// Descent, is the (negated) size of the tails from the baseline.
	// Font height is ascent + descent + line gap.
	// You can request a certain font size, but you may not get an exact font.
	// Height and ascent can be queried. For width, you need to render.

	audio_manager.emplace();
	sprite_effects_sheet = SpriteEffectsSheet{renderer};
	shapes_sheet = ShapesSheet{renderer};
	dynamic_sheet = DynamicSheet{renderer};

	// Load game resources
	resource_dir = resource_path() / "game" / _proxy.game_identifier;
	levels_dir = resource_path() / "game" / _proxy.game_identifier / "levels";

	auto sheets_pb = pb::json_file_to_message<pb::SpriteSheets>(resource_dir / "SpriteSheets.json");
	if (!sheets_pb) {
		throw M2_ERROR(sheets_pb.error());
	}
	sprite_sheets = load_sprite_sheets(*sheets_pb, renderer, _proxy.lightning);
	_sprites =
		load_sprites(sprite_sheets, sheets_pb->text_labels(), *sprite_effects_sheet, renderer, font, _proxy.default_font_size, _proxy.lightning);
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
	if (not std::holds_alternative<std::monostate>(_multi_player_threads)) {
		throw M2_ERROR("Hosting game requires no other multiplayer threads to exist");
	}

	LOG_INFO("Creating server instance...");
	_multi_player_threads.emplace<ServerThreads>();
	LOG_DEBUG("Destroying temporary ServerThread...");
	server_thread().~ServerThread(); // Destruct the default object
	LOG_DEBUG("Temporary ServerThread destroyed, creating real ServerThread...");
	new (&server_thread()) network::ServerThread(type, max_connection_count);
	LOG_DEBUG("Real ServerThread created");

	// Wait until the server is up
	while (not server_thread().is_listening()) {
		SDL_Delay(25);
	}
	// TODO prevent other clients from joining until the host client joins

	LOG_INFO("Server is listening, joining the game as host client...");
	LOG_DEBUG("Destroying temporary HostClientThread...");
	host_client_thread().~HostClientThread(); // Destruct the default object
	LOG_DEBUG("Temporary HostClientThread destroyed, creating real HostClientThread");
	new (&host_client_thread()) network::HostClientThread(type);
	LOG_DEBUG("Real HostClientThread created");

	return {};
}

m2::void_expected m2::Game::join_game(mplayer::Type type, const std::string& addr) {
	if (not std::holds_alternative<std::monostate>(_multi_player_threads)) {
		throw M2_ERROR("Joining game requires no other multiplayer threads to exist");
	}

	_multi_player_threads.emplace<network::RealClientThread>(type, addr);
	return {};
}

void m2::Game::add_bot() {
	if (not is_server()) {
		throw M2_ERROR("Only server can add bots");
	}

	auto it = _bot_threads.emplace(_bot_threads.end());
	LOG_INFO("Joining the game as bot client...");
	LOG_DEBUG("Destroying temporary BotClientThread...");
	it->first.~BotClientThread(); // Destruct the default object
	LOG_DEBUG("Temporary BotClientThread destroyed, creating real BotClientThread");
	new (&it->first) network::BotClientThread(server_thread().type());
	LOG_DEBUG("Real BotClientThread created");

	it->second = -1; // Index is initially unknown
}

m2::network::BotClientThread& m2::Game::find_bot(int receiver_index) {
	auto it = std::find_if(_bot_threads.begin(), _bot_threads.end(), is_second_equals<network::BotClientThread, int>(receiver_index));
	if (it == _bot_threads.end()) {
		throw M2_ERROR("Bot not found");
	}
	return it->first;
}

int m2::Game::total_player_count() {
	if (is_server()) {
		return server_thread().client_count();
	} else if (is_real_client()) {
		return real_client_thread().total_player_count();
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}

int m2::Game::self_index() {
	if (is_server()) {
		return 0;
	} else if (is_real_client()) {
		return real_client_thread().self_index();
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}

int m2::Game::turn_holder_index() {
	if (is_server()) {
		return server_thread().turn_holder_index();
	} else if (is_real_client()) {
		return real_client_thread().turn_holder_index();
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}

bool m2::Game::is_our_turn() {
	if (is_server()) {
		return server_thread().is_our_turn();
	} else if (is_real_client()) {
		return real_client_thread().is_our_turn();
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}

void m2::Game::queue_client_command(const m2g::pb::ClientCommand& cmd) {
	if (is_server()) {
		host_client_thread().queue_client_command(cmd);
	} else if (is_real_client()) {
		real_client_thread().queue_client_command(cmd);
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
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

	auto success = _level->init_multi_player_as_host(level_path_or_blueprint, level_name);
	m2_reflect_unexpected(success);

	// Execute the first server update, which will trigger clients to initialize their levels, but not fully.
	M2_GAME.server_thread().send_server_update();
	// Manually set the HostClientThread state to STARTED, because it doesn't receive ServerUpdates
	M2_GAME.host_client_thread().start_if_ready();
	// If there are bots, we need to handle the first server update
	std::this_thread::sleep_for(std::chrono::seconds(1)); // TODO why? system takes some time to deliver the data to bots, even though they are on the same machine
	for (auto& [bot, index] : _bot_threads) {
		auto server_update = bot.pop_server_update();
		if (not server_update) {
			throw M2_ERROR("Bot hasn't received the ServerUpdate");
		}
		index = server_update->receiver_index();
	}

	// Populate level
	M2G_PROXY.multi_player_level_server_populate(level_name, *_level->_lb);

	// Execute second server update, which will fully initialize client levels.
	M2_GAME.server_thread().send_server_update();
	// Act as if ServerUpdate is received on the server-side as well
	LOG_DEBUG("Calling server-side post_server_update...");
	_proxy.post_server_update();

	// If there are bots, we need to consume the second server update as bots are never the first turn holder.
	std::this_thread::sleep_for(std::chrono::seconds(1));
	for (auto& [bot, index] : _bot_threads) {
		auto server_update = bot.pop_server_update();
		if (not server_update) {
			throw M2_ERROR("Bot hasn't received the ServerUpdate");
		}
		index = server_update->receiver_index();
	}

	return success;
}

m2::void_expected m2::Game::load_multi_player_as_guest(
	const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	reset_state();
	_level.emplace();

	auto success = _level->init_multi_player_as_guest(level_path_or_blueprint, level_name);
	m2_reflect_unexpected(success);

	// Consume the initial ServerUpdate that triggered the level to be initialized
	auto expect_server_update = M2_GAME.real_client_thread().process_server_update();
	m2_reflect_unexpected(expect_server_update);

	return success;
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
		if (ui::Panel::create_and_run_blocking(&ui::console_ui).is_quit()) {
			quit = true;
		}
	}
}

void m2::Game::handle_menu_event() {
	if (events.pop_key_press(Key::MENU)) {
		// Select the correct pause menu
		const ui::PanelBlueprint* pause_menu{};
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
		if (pause_menu && ui::Panel::create_and_run_blocking(pause_menu).is_quit()) {
			quit = true;
		}
	}
}

void m2::Game::handle_hud_events() {
	if (_level->custom_blocking_ui_panel) {
		_level->custom_blocking_ui_panel->handle_events(events)
			.if_any_return([this]() {
				// If the blocking panel returned, remove the state
				_level->custom_blocking_ui_panel.reset();
			});
		// TODO handle quit
		// If there's a blocking UI panel, no events will be delivered to rest of the UI states and the game world
		events.clear();
		// handle_events of others are executed regardless, because there may be timing related actions
	}

	// The order of event handling is the reverse of the drawing order
	for (auto &panel : std::ranges::reverse_view(_level->_custom_nonblocking_ui_panels)) {
		panel.handle_events(events)
			.if_any_return([&panel]() {
				panel = ui::Panel(); // If UI returned, reset the panel. We cannot delete it, the iterator is held by the client
				// TODO returned object is lost, maybe we can store it inside Panel
			});
		// TODO handle quit
	}
	IF(_level->message_box_ui_panel)->handle_events(events);
	IF(_level->left_hud_ui_panel)->handle_events(events);
	IF(_level->right_hud_ui_panel)->handle_events(events);
}

void m2::Game::handle_network_events() {
	// Check if the game ended
	if ((is_server() && host_client_thread().is_shutdown()) || (is_real_client() && real_client_thread().is_shutdown())) {
		_multi_player_threads = std::monostate{};
		_bot_threads.clear();
		// Execute main menu
		if (ui::Panel::create_and_run_blocking(_proxy.main_menu()).is_quit()) {
			quit = true;
		}
	}

	if (is_server()) {
		// Check if a client has disconnected
		if (auto disconnected_client_index = server_thread().disconnected_client()) {
			auto action = _proxy.handle_disconnected_client(*disconnected_client_index);
			// TODO handle
		}

		// Check if a client has misbehaved
		if (auto misbehaved_client_index = server_thread().misbehaved_client()) {
			auto action = _proxy.handle_misbehaving_client(*misbehaved_client_index);
			// TODO handle
		}
	}

	if (is_real_client()) {
		// Check if the client has reconnected and needs to be set as ready
		if (real_client_thread().is_reconnected()) {
			// Set as ready using the same ready_token
			M2_GAME.real_client_thread().set_ready(true);
			// Expect ServerUpdate, handle it normally
		}

		// Check if the client has disconnected
		if (real_client_thread().has_timed_out()) {
			_proxy.handle_disconnection_from_server();
			// TODO handle
		}

		// Check if the server has behaved unexpectedly
		if (real_client_thread().is_server_unrecognized()) {
			_proxy.handle_unrecognized_server();
			// TODO handle
		}
	}
}

void m2::Game::execute_pre_step() {
	for (auto& phy : _level->physics) {
		IF(phy.pre_step)(phy);
	}
	if (is_server()) {
		if (not _bot_threads.empty()) {
			// Check if any of the bots need to handle the ServerUpdate
			for (auto& [bot, _] : _bot_threads) {
				if (auto server_update = bot.pop_server_update()) {
					_proxy.bot_handle_server_update(*server_update);
				}
			}
		}

		// Handle client command
		if (auto client_command = server_thread().pop_turn_holder_command()) {
			auto new_turn_holder = _proxy.handle_client_command(server_thread().turn_holder_index(), client_command->client_command());
			if (new_turn_holder) {
				if (*new_turn_holder < 0) {
					server_thread().shutdown();
					// TODO remove threads, push main menu, maybe do this in handle_network_events?
				} else {
					server_thread().set_turn_holder(*new_turn_holder);
					_server_update_necessary = true;
				}
			}
		}
		// Handle reconnected client
		if (server_thread().has_reconnected_client()) {
			_server_update_necessary = true;
		}

		// Handle server command
		if (auto server_command = host_client_thread().pop_server_command()) {
			_proxy.handle_server_command(*server_command);
		}
		if (not _bot_threads.empty()) {
			// Check if any of the bots need to handle the ServerCommand
			for (auto& [bot, index] : _bot_threads) {
				if (auto server_command = bot.pop_server_command()) {
					_proxy.bot_handle_server_command(*server_command, index);
				}
			}
		}
	} else if (is_real_client()) {
		// Handle server command
		if (auto server_command = real_client_thread().pop_server_command()) {
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
				auto& object = phy.owner();
				auto old_pos = object.position;
				// Update draw list
				object.position = m2::VecF{phy.body->GetPosition()};
				if (old_pos != object.position) {
					_level->draw_list.queue_update(phy.owner_id(), object.position);
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
	if (is_server()) {
		if (_server_update_necessary) {
			LOG_DEBUG("Server update is necessary, sending ServerUpdate...");
			server_thread().send_server_update();
			_server_update_necessary = false; // Unset flag

			// Act as if ServerUpdate is received on the server-side as well
			LOG_DEBUG("Calling server-side post_server_update...");
			_proxy.post_server_update();
		}
	} else if (is_real_client()) {
		// Handle ServerUpdate
		auto expect_success = real_client_thread().process_server_update();
		m2_succeed_or_throw_error(expect_success);
		if (expect_success.value()) {
			LOG_DEBUG("Calling client-side post_server_update...");
			_proxy.post_server_update();
		}
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
	IF(_level->left_hud_ui_panel)->update_contents(_delta_time_s);
	IF(_level->right_hud_ui_panel)->update_contents(_delta_time_s);
	IF(_level->message_box_ui_panel)->update_contents(_delta_time_s);
	for (auto &panel : _level->_custom_nonblocking_ui_panels) {
		panel.update_contents(_delta_time_s)
			.if_any_return([&panel]() {
				// If returned, reset the panel. We cannot delete it, the iterator is held by the client
				panel = ui::Panel{};
				// TODO returned object is lost, maybe we can store it inside Panel
			});
		// TODO handle quit
	}
	IF(_level->custom_blocking_ui_panel)->update_contents(_delta_time_s);
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
	IF(_level->left_hud_ui_panel)->draw();
	IF(_level->right_hud_ui_panel)->draw();
	IF(_level->message_box_ui_panel)->draw();
	for (auto &panel : _level->_custom_nonblocking_ui_panels) {
		panel.draw();
	}
	IF(_level->custom_blocking_ui_panel)->draw();
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
	_dims = Dimensions{game_height_m == 0 ? _dims.height_m : game_height_m, window_width, window_height, _proxy.game_aspect_ratio_mul, _proxy.game_aspect_ratio_div};
}

void m2::Game::set_zoom(const float game_height_multiplier) {
	recalculate_dimensions(
		_dims.window.w, _dims.window.h, iround(static_cast<float>(_dims.height_m) * game_height_multiplier));
	if (_level) {
		IF(_level->left_hud_ui_panel)->update_positions();
		IF(_level->right_hud_ui_panel)->update_positions();
		IF(_level->message_box_ui_panel)->update_positions();
		for (auto &panel : _level->_custom_nonblocking_ui_panels) {
			panel.update_positions();
		}
		IF (_level->custom_blocking_ui_panel)->update_positions();
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

m2::sdl::TextureUniquePtr m2::Game::draw_game_to_texture(m2::VecF camera_position) {
	// Temporarily change camera position
	auto prev_camera_position = level().camera()->position;
	level().camera()->position = camera_position;

	// Create an empty render target
	auto render_target = sdl::create_drawable_texture_of_screen_size();
	// Temporarily change render target
	auto prev_render_target = SDL_GetRenderTarget(renderer);
	SDL_SetRenderTarget(renderer, render_target.get());

	// Draw
	clear_back_buffer();
	draw_background();
	draw_foreground();
	draw_lights();
	draw_envelopes();

	// Reinstate old render target
	SDL_SetRenderTarget(renderer, prev_render_target);

	// Reinstate old camera position
	level().camera()->position = prev_camera_position;

	return render_target;
}

void m2::Game::recalculate_directional_audio() {
	if (_level->left_listener || _level->right_listener) {
		// Loop over sounds
		for (auto& sound_emitter : _level->sound_emitters) {
			const auto& sound_position = sound_emitter.owner().position;
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

void m2::Game::enable_dimming_with_exceptions(std::set<ObjectId> exceptions) {
	_dimming_exceptions = std::move(exceptions);
}

void m2::Game::disable_dimming_with_exceptions() {
	_dimming_exceptions.reset();
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
