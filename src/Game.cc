#include <m2/Game.h>
#include <m2/Log.h>

#include <Level.pb.h>
#include <SDL2/SDL_image.h>
#include <m2/Error.h>
#include <m2/Object.h>
#include <m2/video/Sprite.h>
#include <m2/String.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/level_editor/Ui.h>
#include <m2/sdl/Detail.h>
#include <m2/sheet_editor/Ui.h>
#include <m2/FileSystem.h>
#include <filesystem>
#include <ranges>
#include "m2/component/Graphic.h"
#include <m2/ui/UiAction.h>

m2::Game* m2::Game::_instance;

void m2::Game::CreateInstance() {
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

void m2::Game::DestroyInstance() {
	DEBUG_FN();
	delete _instance;
	_instance = nullptr;
}

m2::Game::Game() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		throw M2_ERROR("SDL_Init error: " + std::string{SDL_GetError()});
	}
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		throw M2_ERROR("IMG_Init error: " + std::string{IMG_GetError()});
	}
	if (TTF_Init() != 0) {
		throw M2_ERROR("TTF_Init error: " + std::string{TTF_GetError()});
	}

	// Default Metal backend is slow in 2.5D mode, while drawing the rectangle debug shapes
	if (SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl") == false) {
		LOG_WARN("Failed to set opengl as render hint");
	}
	// Use the driver line API
	if (SDL_SetHint(SDL_HINT_RENDER_LINE_METHOD, "2") == false) {
		LOG_WARN("Failed to set line render method");
	}

	auto minimumWindowDims = GameDimensionsManager::EstimateMinimumWindowDimensions(_proxy.gamePpm, _proxy.defaultGameHeightM);
	if ((window = SDL_CreateWindow(_proxy.game_friendly_name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		minimumWindowDims.x, minimumWindowDims.y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == nullptr) {
		throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_SetWindowMinimumSize(window, minimumWindowDims.x, minimumWindowDims.y);
	SDL_StopTextInput(); // Text input begins activated (sometimes)

	cursor = SdlUtils_CreateCursor();
	SDL_SetCursor(cursor);
	if ((pixel_format = SDL_GetWindowPixelFormat(window)) == SDL_PIXELFORMAT_UNKNOWN) {
		throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
	}

	if (_proxy.areGraphicsPixelated) {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	} else {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	}

	if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) ==
		nullptr) {  // TODO: SDL_RENDERER_PRESENTVSYNC
		throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
	}
	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	LOG_INFO("Renderer", info.name);

	_dimensionsManager.emplace(renderer, _proxy.gamePpm, _proxy.gameAspectRatioMul, _proxy.gameAspectRatioDiv);

	SDL_Surface* lightSurface = IMG_Load((resource_path() / "RadialGradient-WhiteBlack.png").string().c_str());
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
	if ((font = TTF_OpenFont((resource_path() / _proxy.default_font_path).string().c_str(), _proxy.default_font_size)) == nullptr) {
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
	// Font size, is the size of the letter from the baseline (ascent).
	// Descent, is the (negated) size of the tails from the baseline.
	// Font height is ascent + descent + line gap.
	// You can request a certain font size, but you may not get an exact font.
	// Height and ascent can be queried. For width, you need to render.
	_font_letter_width_to_height_ratio = Rational{font_letter_width, font_letter_height};
	_textLabelCache.emplace(renderer, font);
	_shapeCache.emplace(renderer);

	audio_manager.emplace();
	spriteEffectsSheet = SpriteEffectsSheet{renderer};

	// Load game resources
	resource_dir = resource_path() / "game" / _proxy.game_identifier;
	levels_dir = resource_path() / "game" / _proxy.game_identifier / "levels";

	auto sheets_pb = pb::json_file_to_message<pb::SpriteSheets>(resource_dir / "SpriteSheets.json");
	if (!sheets_pb) {
		throw M2_ERROR(sheets_pb.error());
	}
	spriteSheets = SpriteSheet::LoadSpriteSheets(*sheets_pb, renderer, _proxy.lightning);
	_sprites = LoadSprites(spriteSheets, sheets_pb->text_labels(), *spriteEffectsSheet, _proxy.lightning);
	LOG_INFO("Loaded sprites", _sprites.size());
	auto backgroundSprites = _sprites | std::views::filter(IsSpriteBackgroundTile) | std::views::transform(ToSpriteType);
	level_editor_background_sprites = std::vector<m2g::pb::SpriteType>{backgroundSprites.begin(), backgroundSprites.end()};
	LOG_INFO("Loaded level editor background sprites", level_editor_background_sprites.size());
	object_main_sprites = ListLevelEditorObjectSprites(resource_dir / "Objects.json");
	LOG_INFO("Loaded objects", object_main_sprites.size());
	named_items = pb::LUT<m2::pb::Item, NamedItem>::load(resource_dir / "Items.json", &m2::pb::Items::items);
	LOG_INFO("Loaded named items", named_items.size());
	animations = pb::LUT<m2::pb::Animation, Animation>::load(resource_dir / "Animations.json", &m2::pb::Animations::animations);
	LOG_INFO("Loaded animations", animations.size());
	songs = pb::LUT<m2::pb::Song, Song>::load(resource_dir / "Songs.json", &m2::pb::Songs::songs);
	LOG_INFO("Loaded songs", songs.size());
}

m2::Game::~Game() {
	_level.reset();
	audio_manager.reset();
	SDL_DestroyRenderer(renderer);
	SDL_FreeCursor(cursor);
	SDL_DestroyWindow(window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

m2::void_expected m2::Game::HostGame(mplayer::Type type, unsigned max_connection_count) {
	if (not std::holds_alternative<std::monostate>(_multi_player_threads)) {
		throw M2_ERROR("Hosting game requires no other multiplayer threads to exist");
	}

	LOG_INFO("Creating server instance...");
	_multi_player_threads.emplace<ServerThreads>();
	LOG_DEBUG("Destroying temporary ServerThread...");
	ServerThread().~ServerThread(); // Destruct the default object
	LOG_DEBUG("Temporary ServerThread destroyed, creating real ServerThread...");
	new (&ServerThread()) network::ServerThread(type, max_connection_count);
	LOG_DEBUG("Real ServerThread created");

	// Wait until the server is up
	while (not ServerThread().is_listening()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	// TODO prevent other clients from joining until the host client joins

	LOG_INFO("Server is listening, joining the game as host client...");
	LOG_DEBUG("Destroying temporary HostClientThread...");
	HostClientThread().~HostClientThread(); // Destruct the default object
	LOG_DEBUG("Temporary HostClientThread destroyed, creating real HostClientThread");
	new (&HostClientThread()) network::HostClientThread(type);
	LOG_DEBUG("Real HostClientThread created");

	return {};
}

m2::void_expected m2::Game::JoinGame(mplayer::Type type, const std::string& addr) {
	if (not std::holds_alternative<std::monostate>(_multi_player_threads)) {
		throw M2_ERROR("Joining game requires no other multiplayer threads to exist");
	}

	_multi_player_threads.emplace<network::RealClientThread>(type, addr);
	return {};
}
void m2::Game::LeaveGame() {
	_multi_player_threads = std::monostate{};
}

bool m2::Game::AddBot() {
	if (not IsServer()) {
		throw M2_ERROR("Only server can add bots");
	}

	// Create an instance at the end of the list
	auto it = _bot_threads.emplace(_bot_threads.end());
	LOG_INFO("Joining the game as bot client...");

	LOG_DEBUG("Destroying temporary BotClientThread...");
	it->first.~BotClientThread(); // Destruct the default object
	LOG_DEBUG("Temporary BotClientThread destroyed, creating real BotClientThread");

	new (&it->first) network::BotClientThread(ServerThread().type());
	LOG_DEBUG("Real BotClientThread created");

	if (it->first.is_active()) {
		it->second = -1; // Index is initially unknown
		return true;
	} else {
		LOG_WARN("BotClientThread failed to connect, destroying client");
		_bot_threads.erase(it);
		return false;
	}
}

m2::network::BotClientThread& m2::Game::FindBot(int receiver_index) {
	auto it = std::find_if(_bot_threads.begin(), _bot_threads.end(), is_second_equals<network::BotClientThread, int>(receiver_index));
	if (it == _bot_threads.end()) {
		throw M2_ERROR("Bot not found");
	}
	return it->first;
}

int m2::Game::TotalPlayerCount() {
	if (IsServer()) {
		return ServerThread().client_count();
	} else if (IsRealClient()) {
		return RealClientThread().total_player_count();
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}

int m2::Game::SelfIndex() {
	if (IsServer()) {
		return 0;
	} else if (IsRealClient()) {
		return RealClientThread().self_index();
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}

int m2::Game::TurnHolderIndex() {
	if (IsServer()) {
		return ServerThread().turn_holder_index();
	} else if (IsRealClient()) {
		return RealClientThread().turn_holder_index();
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}

bool m2::Game::IsOurTurn() {
	if (IsServer()) {
		return ServerThread().is_our_turn();
	} else if (IsRealClient()) {
		if (RealClientThread().is_started()) {
			return RealClientThread().is_our_turn();
		} else {
			return false;
		}
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}

void m2::Game::QueueClientCommand(const m2g::pb::ClientCommand& cmd) {
	if (IsServer()) {
		HostClientThread().queue_client_command(cmd);
	} else if (IsRealClient()) {
		RealClientThread().queue_client_command(cmd);
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}

m2::void_expected m2::Game::LoadSinglePlayer(
	const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	ResetState();
	_level.emplace();
	return _level->InitSinglePlayer(level_path_or_blueprint, level_name);
}

m2::void_expected m2::Game::LoadMultiPlayerAsHost(
	const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	ResetState();
	_level.emplace();

	auto success = _level->InitMultiPlayerAsHost(level_path_or_blueprint, level_name);
	m2_reflect_unexpected(success);

	// Execute the first server update, which will trigger clients to initialize their levels, but not fully.
	M2_GAME.ServerThread().send_server_update();
	// Manually set the HostClientThread state to STARTED, because it doesn't receive ServerUpdates
	M2_GAME.HostClientThread().start_if_ready();
	// If there are bots, we need to handle the first server update
	LOG_DEBUG("Waiting 1s until the first server update is delivered to bots");
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
	_lastSentOrReceivedServerUpdateSequenceNo = M2_GAME.ServerThread().send_server_update();
	// Act as if ServerUpdate is received on the server-side as well
	LOG_DEBUG("Calling server-side post_server_update...");
	_proxy.post_server_update(*_lastSentOrReceivedServerUpdateSequenceNo, false);

	// If there are bots, we need to consume the second server update as bots are never the first turn holder.
	LOG_DEBUG("Waiting 1s until the second server update is delivered to bots");
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

m2::void_expected m2::Game::LoadMultiPlayerAsGuest(
	const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	ResetState();
	_level.emplace();

	auto success = _level->InitMultiPlayerAsGuest(level_path_or_blueprint, level_name);
	m2_reflect_unexpected(success);

	// Consume the initial ServerUpdate that triggered the level to be initialized
	auto expect_server_update = M2_GAME.RealClientThread().process_server_update();
	m2_reflect_unexpected(expect_server_update);
	_lastSentOrReceivedServerUpdateSequenceNo = expect_server_update->second;
	m2_return_unexpected_message_unless(expect_server_update->first == network::ServerUpdateStatus::PROCESSED,
			"Unexpected ServerUpdate status");
	return {};
}

m2::void_expected m2::Game::LoadLevelEditor(const std::string& level_resource_path) {
	_level.reset();
	ResetState();
	_level.emplace();
	return _level->InitLevelEditor(level_resource_path);
}

m2::void_expected m2::Game::LoadPixelEditor(
	const std::string& image_resource_path, const int x_offset, const int y_offset) {
	_level.reset();
	ResetState();
	_level.emplace();
	return _level->InitPixelEditor(image_resource_path, x_offset, y_offset);
}

m2::void_expected m2::Game::LoadSheetEditor() {
	_level.reset();
	ResetState();
	_level.emplace();
	return _level->InitSheetEditor(resource_dir / "SpriteSheets.json");
}

m2::void_expected m2::Game::LoadBulkSheetEditor() {
	_level.reset();
	ResetState();
	_level.emplace();
	return _level->InitBulkSheetEditor(resource_dir / "SpriteSheets.json");
}

void m2::Game::ResetState() { events.clear(); }

void m2::Game::HandleQuitEvent() {
	if (events.pop_quit()) {
		quit = true;
	}
}

void m2::Game::HandleWindowResizeEvent() {
	if (events.pop_window_resize()) {
		OnWindowResize();
	}
}

void m2::Game::HandleConsoleEvent() {
	if (events.pop_key_press(Key::CONSOLE)) {
		if (UiPanel::create_and_run_blocking(&console_ui).IsQuit()) {
			quit = true;
		}
	}
}

void m2::Game::HandleMenuEvent() {
	if (events.pop_key_press(Key::MENU)) {
		// Select the correct pause menu
		const UiPanelBlueprint* PauseMenuBlueprint{};
		if (std::holds_alternative<splayer::State>(Level().stateVariant)) {
			PauseMenuBlueprint = _proxy.PauseMenuBlueprint();
		} else if (std::holds_alternative<level_editor::State>(Level().stateVariant)) {
			PauseMenuBlueprint = nullptr;
		} else if (std::holds_alternative<sheet_editor::State>(Level().stateVariant)) {
			PauseMenuBlueprint = &sheet_editor_main_menu;
		} else if (std::holds_alternative<bulk_sheet_editor::State>(Level().stateVariant)) {
			PauseMenuBlueprint = nullptr;
		}

		// Execute pause menu if found, exit if QUIT is returned
		if (PauseMenuBlueprint && UiPanel::create_and_run_blocking(PauseMenuBlueprint).IsQuit()) {
			quit = true;
		}
	}
}

void m2::Game::HandleHudEvents() {
	if (_level->_semiBlockingUiPanel) {
		_level->_semiBlockingUiPanel->HandleEvents(events, _level->isPanning)
				.IfQuit([this]() {
					quit = true;
				})
				.IfAnyReturn([this](const ReturnBase&) {
					// The return object is discarded. Remove the state.
					_level->_semiBlockingUiPanel.reset();
				});

		// After semi-blocking UI, events are cleared to prevent the game objects from receiving it.
		events.clear();
		// Handling of events of other UI panels are also skipped.
	} else {
		// Mouse hover UI panel doesn't receive events, but based on the mouse position, it may be moved
		IF(_level->_mouseHoverUiPanel)->SetTopLeftPosition(_level->CalculateMouseHoverUiPanelTopLeftPosition());

		// The order of event handling is the reverse of the drawing order
		for (auto &panel : std::ranges::reverse_view(_level->_customNonblockingUiPanels)) {
			auto action{panel.HandleEvents(events, _level->isPanning)};
			action.IfQuit([this]() { quit = true; });
			if (auto anyReturnContainer = action.ExtractAnyReturnContainer()) {
				// If UI returned, kill the panel. We cannot delete it yet, the iterator is held by the client, but we
				// can replace it with a killed object that can hold the AnyReturnContainer instead.
				panel.KillWithReturnValue(std::move(*anyReturnContainer));
			}
		}
		IF(_level->_messageBoxUiPanel)->HandleEvents(events, _level->isPanning);
		IF(_level->_rightHudUiPanel)->HandleEvents(events, _level->isPanning);
		IF(_level->_leftHudUiPanel)->HandleEvents(events, _level->isPanning);
	}
}

void m2::Game::HandleNetworkEvents() {
	// Check if the game ended
	if ((IsServer() && ServerThread().is_shutdown()) || (IsRealClient() && RealClientThread().is_shutdown())) {
		_level.reset();
		ResetState();
		_multi_player_threads = std::monostate{};
		_bot_threads.clear();
		// Execute main menu
		if (UiPanel::create_and_run_blocking(_proxy.MainMenuBlueprint()).IsQuit()) {
			quit = true;
		}
	} else if (IsServer()) {
		// Check if a client has disconnected
		if (auto disconnected_client_index = ServerThread().disconnected_client()) {
			MAYBE auto action = _proxy.handle_disconnected_client(*disconnected_client_index);
			// TODO handle
		}

		// Check if a client has misbehaved
		if (auto misbehaved_client_index = ServerThread().misbehaved_client()) {
			MAYBE auto action = _proxy.handle_misbehaving_client(*misbehaved_client_index);
			// TODO handle
		}
	} else if (IsRealClient()) {
		// Check if the client has reconnected and needs to be set as ready
		if (RealClientThread().is_reconnected()) {
			// Set as ready using the same ready_token
			M2_GAME.RealClientThread().set_ready(true);
			// Expect ServerUpdate, handle it normally
		}

		// Check if the client has disconnected
		if (RealClientThread().has_reconnection_timed_out()) {
			_proxy.handle_disconnection_from_server();
			// TODO handle
		}

		// Check if the server has behaved unexpectedly
		if (RealClientThread().is_server_unrecognized()) {
			_proxy.handle_unrecognized_server();
			// TODO handle
		}
	}
}

void m2::Game::ExecutePreStep() {
	_proxy.OnPreStep();
	ExecuteDeferredActions();
	for (auto& phy : _level->physics) {
		IF(phy.preStep)(phy);
	}
	if (IsServer()) {
		if (not _bot_threads.empty()) {
			// Check if any of the bots need to handle the ServerUpdate
			for (auto& [bot, _] : _bot_threads) {
				if (auto server_update = bot.pop_server_update()) {
					_proxy.bot_handle_server_update(*server_update);
				}
			}
		}

		// Handle client command
		if (auto client_command = ServerThread().pop_turn_holder_command()) {
			auto new_turn_holder = _proxy.handle_client_command(ServerThread().turn_holder_index(), client_command->second.client_command());
			if (new_turn_holder) {
				if (*new_turn_holder < 0) {
					_server_update_necessary = true;
					_server_update_with_shutdown = true;
				} else {
					ServerThread().set_turn_holder(*new_turn_holder);
					_server_update_necessary = true;
				}
			}
		}
		// Handle reconnected client
		if (ServerThread().has_reconnected_client()) {
			_server_update_necessary = true;
		}

		// Handle server command
		if (auto server_command = HostClientThread().pop_server_command()) {
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
	} else if (IsRealClient()) {
		// Handle server command
		if (auto server_command = RealClientThread().pop_server_command()) {
			_proxy.handle_server_command(*server_command);
		}
	}
}

void m2::Game::UpdateCharacters() {
	for (auto& character : _level->characters) {
		auto& chr = ToCharacterBase(character);
		chr.AutomaticUpdate();
	}
	for (auto& character : _level->characters) {
		auto& chr = ToCharacterBase(character);
		IF(chr.update)(chr);
	}
}

void m2::Game::ExecuteStep() {
	if (_level->world) {
		LOGF_TRACE("Stepping world %f seconds...", phy_period);
		_level->world->Step(phy_period, velocity_iterations, position_iterations);
		_level->World2().Integrate();
		LOG_TRACE("World stepped");
		// Update positions
		for (auto& phy : _level->physics) {
			if (phy.body) {
				auto& object = phy.owner();
				auto old_pos = object.position;
				object.position = m2::VecF{phy.body->GetPosition()};
				object.orientation = phy.body->GetAngle();
				// Update draw list
				if (old_pos != object.position) {
					_level->drawList.QueueUpdate(phy.owner_id(), object.position);
				}
			} else if (phy.rigidBodyIndex) {
				const auto& rigidBody = _level->World2().GetRigidBody(*phy.rigidBodyIndex);
				auto& object = phy.owner();
				auto oldPosition = object.position;
				auto newPosition = rigidBody.PositionOfCenterOfMass();
				object.position.x = newPosition.X().ToFloat();
				object.position.y = newPosition.Y().ToFloat();
				object.orientation = rigidBody.OrientationAboutCenterOfMass().ToFloat();
				// Update draw list
				if (oldPosition != object.position) {
					_level->drawList.QueueUpdate(phy.owner_id(), object.position);
				}
			}
		}
	}
	// Re-sort draw list
	_level->drawList.Update();
	if (not _proxy.world_is_static) {
		// If the world is NOT static, the pathfinder's cache should be cleared, because the objects might have been
		// moved
		_level->pathfinder->clear_cache();
	}
}

void m2::Game::ExecutePostStep() {
	if (IsServer()) {
		if (_server_update_necessary) {
			LOG_DEBUG("Server update is necessary, sending ServerUpdate...");
			_lastSentOrReceivedServerUpdateSequenceNo = ServerThread().send_server_update(_server_update_with_shutdown);
			_server_update_necessary = false; // Unset flag

			// Act as if ServerUpdate is received on the server-side as well
			LOG_DEBUG("Calling server-side post_server_update...");
			_proxy.post_server_update(*_lastSentOrReceivedServerUpdateSequenceNo, _server_update_with_shutdown);

			// Shutdown the game if necessary
			if (_server_update_with_shutdown) {
				if (not ServerThread().is_shutdown()) {
					throw M2_ERROR("Server should have shutdown itself");
				}
				_server_update_with_shutdown = false;
				// Game will be restarted in handle_network_events
			}
		}
	} else if (IsRealClient()) {
		// Handle ServerUpdate
		auto status = RealClientThread().process_server_update();
		m2_succeed_or_throw_error(status);

		if (status->first == network::ServerUpdateStatus::PROCESSED || status->first == network::ServerUpdateStatus::PROCESSED_SHUTDOWN) {
			_lastSentOrReceivedServerUpdateSequenceNo = status->second;
			LOG_DEBUG("Calling client-side post_server_update...");
			_proxy.post_server_update(*_lastSentOrReceivedServerUpdateSequenceNo, status->first == network::ServerUpdateStatus::PROCESSED_SHUTDOWN);
		}
		if (status->first == network::ServerUpdateStatus::PROCESSED_SHUTDOWN) {
			RealClientThread().shutdown();
			// Game will be restarted in handle_network_events
		}
	}

	_proxy.OnPostStep();
	ExecuteDeferredActions();
	for (auto& phy : _level->physics) {
		IF(phy.postStep)(phy);
	}
}

void m2::Game::UpdateSounds() {
	for (auto& sound_emitter : _level->soundEmitters) {
		IF(sound_emitter.update)(sound_emitter);
	}
}

void m2::Game::ExecutePreDraw() {
	for (auto& gfx : _level->graphics) {
		if (gfx.enabled) {
			IF(gfx.preDraw)(gfx);
		}
	}
}

void m2::Game::UpdateHudContents() {
	// TODO handle returned actions
	IF(_level->_leftHudUiPanel)->UpdateContents(_delta_time_s);
	IF(_level->_rightHudUiPanel)->UpdateContents(_delta_time_s);
	IF(_level->_messageBoxUiPanel)->UpdateContents(_delta_time_s);
	for (auto &panel : _level->_customNonblockingUiPanels) {
		auto action{panel.UpdateContents(_delta_time_s)};
		action.IfQuit([this]() { quit = true; });
		if (auto anyReturnContainer = action.ExtractAnyReturnContainer()) {
			// If UI returned, kill the panel. We cannot delete it yet, the iterator is held by the client, but we
			// can replace it with a killed object that can hold the AnyReturnContainer instead.
			panel.KillWithReturnValue(std::move(*anyReturnContainer));
		}
	}
	IF(_level->_mouseHoverUiPanel)->UpdateContents(_delta_time_s);
	if (_level->_semiBlockingUiPanel) {
		_level->_semiBlockingUiPanel->UpdateContents(_delta_time_s)
				.IfQuit([this]() {
					quit = true;
				})
				.IfAnyReturn([this](const ReturnBase&) {
					// The return object is discarded. Remove the state.
					_level->_semiBlockingUiPanel.reset();
				});
	}
}

void m2::Game::ClearBackBuffer() const {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

namespace {
	void draw_one_background_layer(m2::Pool<m2::Graphic>& terrainGraphics) {
		for (auto& gfx : terrainGraphics) {
			if (gfx.enabled && gfx.draw) {
				IF(gfx.onDraw)(gfx);
			}
		}
	}

	void draw_all_background_layers(m2::Level& level) {
		// Draw all background layers
		for (auto& terrainGraphics : std::ranges::reverse_view(level.terrainGraphics)) {
			draw_one_background_layer(terrainGraphics);
		}
	}
}  // namespace

void m2::Game::DrawBackground() {
	if (std::holds_alternative<level_editor::State>(_level->stateVariant)) {
		if (const auto& rightHudName = _level->RightHud()->Name();
				rightHudName == "PaintBgRightHud" || rightHudName == "SampleBgRightHud" || rightHudName == "SelectBgRightHud") {
			const auto& levelEditorState = std::get<level_editor::State>(_level->stateVariant);
			draw_one_background_layer(_level->terrainGraphics[I(levelEditorState.GetSelectedBackgroundLayer())]);
		} else {
			draw_all_background_layers(*_level);
		}
	} else {
		draw_all_background_layers(*_level);
	}
}

void m2::Game::DrawForeground() {
	for (const auto& gfx_id : _level->drawList) {
		auto& gfx = _level->graphics[gfx_id];
		if (gfx.enabled && gfx.draw) {
			IF(gfx.onDraw)(gfx);
		}
	}
}

void m2::Game::DrawLights() {
	for (auto& light : _level->lights) {
		IF(light.onDraw)(light);
	}
}

void m2::Game::ExecutePostDraw() {
	for (auto& gfx : _level->graphics) {
		if (gfx.enabled) {
			IF(gfx.postDraw)(gfx);
		}
	}
}

void m2::Game::DebugDraw() {
#ifdef DEBUG
	for (auto& phy : _level->physics) {
		IF(phy.onDebugDraw)(phy);
	}

	if (IsProjectionTypePerspective(_level->ProjectionType())) {
		SDL_SetRenderDrawColor(M2_GAME.renderer, 255, 255, 255, 127);
		for (int y = 0; y < 20; ++y) {
			for (int x = 0; x < 20; ++x) {
				m3::VecF p = {x, y, 0};
				if (const auto projected_p = ScreenOriginToProjectionAlongCameraPlaneDstpx(p); projected_p) {
					SDL_RenderDrawPointF(M2_GAME.renderer, projected_p->x, projected_p->y);
				}
			}
		}
	}
#endif
}

void m2::Game::DrawHud() {
	IF(_level->_leftHudUiPanel)->Draw();
	IF(_level->_rightHudUiPanel)->Draw();
	IF(_level->_messageBoxUiPanel)->Draw();
	for (auto &panel : _level->_customNonblockingUiPanels) {
		panel.Draw();
	}
	IF(_level->_mouseHoverUiPanel)->Draw();
	IF(_level->_semiBlockingUiPanel)->Draw();
}

void m2::Game::DrawEnvelopes() const {
	SDL_Rect sdl_rect{};

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	sdl_rect = static_cast<SDL_Rect>(_dimensionsManager->TopEnvelope());
	SDL_RenderFillRect(renderer, &sdl_rect);
	sdl_rect = static_cast<SDL_Rect>(_dimensionsManager->BottomEnvelope());
	SDL_RenderFillRect(renderer, &sdl_rect);
	sdl_rect = static_cast<SDL_Rect>(_dimensionsManager->LeftEnvelope());
	SDL_RenderFillRect(renderer, &sdl_rect);
	sdl_rect = static_cast<SDL_Rect>(_dimensionsManager->RightEnvelope());
	SDL_RenderFillRect(renderer, &sdl_rect);
}

void m2::Game::FlipBuffers() const { SDL_RenderPresent(renderer); }

void m2::Game::OnWindowResize() {
	_dimensionsManager->OnWindowResize();
	if (_level) {
		IF(_level->_leftHudUiPanel)->RecalculateRects();
		IF(_level->_rightHudUiPanel)->RecalculateRects();
		IF(_level->_messageBoxUiPanel)->RecalculateRects();
		for (auto &panel : _level->_customNonblockingUiPanels) {
			panel.RecalculateRects();
		}
		IF(_level->_mouseHoverUiPanel)->RecalculateRects();
		IF (_level->_semiBlockingUiPanel)->RecalculateRects();

		// Clear text label rectangles so that they are regenerated with new size
		for (auto& gfx : _level->graphics) {
			gfx.textLabelRect = {};
		}
		for (auto& terrainGraphics : std::ranges::reverse_view(_level->terrainGraphics)) {
			for (auto& gfx : terrainGraphics) {
				gfx.textLabelRect = {};
			}
		}
	}
}
void m2::Game::SetScale(const float scale) {
	_dimensionsManager->SetScale(scale);
	if (_level) {
		// Clear text label rectangles so that they are regenerated with new size
		for (auto& gfx : _level->graphics) {
			gfx.textLabelRect = {};
		}
		for (auto& terrainGraphics : std::ranges::reverse_view(_level->terrainGraphics)) {
			for (auto& gfx : terrainGraphics) {
				gfx.textLabelRect = {};
			}
		}
	}
}
void m2::Game::SetGameHeightM(const float heightM) {
	_dimensionsManager->SetGameHeightM(heightM);
	if (_level) {
		// Clear text label rectangles so that they are regenerated with new size
		for (auto& gfx : _level->graphics) {
			gfx.textLabelRect = {};
		}
		for (auto& terrainGraphics : std::ranges::reverse_view(_level->terrainGraphics)) {
			for (auto& gfx : terrainGraphics) {
				gfx.textLabelRect = {};
			}
		}
	}
}

void m2::Game::ForEachSprite(const std::function<bool(m2g::pb::SpriteType, const Sprite&)>& op) const {
	for (int i = 0; i < pb::enum_value_count<m2g::pb::SpriteType>(); ++i) {
		auto type = pb::enum_value<m2g::pb::SpriteType>(i);
		const auto& spriteOrTextLabel = GetSpriteOrTextLabel(type);
		if (std::holds_alternative<Sprite>(spriteOrTextLabel) && not op(type, std::get<Sprite>(spriteOrTextLabel))) {
			return;
		}
	}
}

void m2::Game::ForEachNamedItem(const std::function<bool(m2g::pb::ItemType, const NamedItem&)>& op) const {
	for (int i = 0; i < pb::enum_value_count<m2g::pb::ItemType>(); ++i) {
		auto type = pb::enum_value<m2g::pb::ItemType>(i);
		if (!op(type, GetNamedItem(type))) {
			return;
		}
	}
}

const m2::VecF& m2::Game::MousePositionWorldM() const {
	if (not _mouse_position_world_m) {
		RecalculateMousePosition();
	}
	return *_mouse_position_world_m;
}

const m2::VecF& m2::Game::ScreenCenterToMousePositionM() const {
	if (not _screen_center_to_mouse_position_m) {
		RecalculateMousePosition();
	}
	return *_screen_center_to_mouse_position_m;
}

m2::sdl::TextureUniquePtr m2::Game::DrawGameToTexture(m2::VecF camera_position) {
	// Temporarily change camera position
	auto prev_camera_position = Level().Camera()->position;
	Level().Camera()->position = camera_position;

	// Create an empty render target
	auto render_target = sdl::create_drawable_texture_of_screen_size();
	// Temporarily change render target
	auto prev_render_target = SDL_GetRenderTarget(renderer);
	SDL_SetRenderTarget(renderer, render_target.get());

	// Draw
	ClearBackBuffer();
	DrawBackground();
	DrawForeground();
	DrawLights();
	DrawEnvelopes();

	// Reinstate old render target
	SDL_SetRenderTarget(renderer, prev_render_target);

	// Reinstate old camera position
	Level().Camera()->position = prev_camera_position;

	return render_target;
}
bool m2::Game::IsMouseOnAnyUiPanel() const {
	// If semi-blocking UI panel is action, we act as if the UI panel covers the whole screen
	if (_level->_semiBlockingUiPanel) {
		return true;
	}
	// Otherwise, check if the mouse is on top of the other UI panels known to Level
	const auto mouse_position = events.mouse_position();
	if (_level->_leftHudUiPanel && _level->_leftHudUiPanel->Rect().contains(mouse_position)) {
		return true;
	}
	if (_level->_rightHudUiPanel && _level->_rightHudUiPanel->Rect().contains(mouse_position)) {
		return true;
	}
	if (_level->_messageBoxUiPanel && _level->_messageBoxUiPanel->Rect().contains(mouse_position)) {
		return true;
	}
	for (auto &panel : _level->_customNonblockingUiPanels) {
		if (panel.Rect().contains(mouse_position)) {
			return true;
		}
	}
	// Ignore mouse hover panel, as the mouse can't intersect with it.
	return false;
}

void m2::Game::RecalculateDirectionalAudio() {
	if (_level->leftListener || _level->rightListener) {
		// Loop over sounds
		for (auto& sound_emitter : _level->soundEmitters) {
			const auto& sound_position = sound_emitter.owner().position;
			// Loop over playbacks
			for (const auto playback_id : sound_emitter.playbacks) {
				if (!audio_manager->HasPlayback(playback_id)) {
					continue;  // Playback may have finished (if it's ONCE)
				}
				// Left listener
				const auto left_volume =
					_level->leftListener ? _level->leftListener->VolumeOf(sound_position) : 0.0f;
				audio_manager->SetPlaybackLeftVolume(playback_id, left_volume);
				// Right listener
				const auto right_volume =
					_level->rightListener ? _level->rightListener->VolumeOf(sound_position) : 0.0f;
				audio_manager->SetPlaybackRightVolume(playback_id, right_volume);
			}
		}
	}
}

void m2::Game::AddDeferredAction(const std::function<void(void)>& action) {
	_level->deferredActions.push(action);
}

void m2::Game::ExecuteDeferredActions() {
	// Execute deferred actions one by one. A deferred action may insert another deferred action into the queue. Thus we
	// cannot iterate over the queue, we must pop one by one.
	while (not _level->deferredActions.empty()) {
		_level->deferredActions.front()();
		_level->deferredActions.pop();
	}
}

void m2::Game::RecalculateMousePosition() const {
	const auto mouse_position = events.mouse_position();
	const auto screen_center_to_mouse_position_px =
		VecI{mouse_position.x - (Dimensions().WindowDimensions().x / 2), mouse_position.y - (Dimensions().WindowDimensions().y / 2)};
	_screen_center_to_mouse_position_m = VecF{
		F(screen_center_to_mouse_position_px.x) / Dimensions().OutputPixelsPerMeter(), F(screen_center_to_mouse_position_px.y) / Dimensions().OutputPixelsPerMeter()};

	if (IsProjectionTypePerspective(_level->ProjectionType())) {
		// Mouse moves on the plane centered at the player looking towards the camera
		// Find m3::VecF of the mouse position in the world starting from the player position
		const auto sin_of_player_to_camera_angle = M2_LEVEL.CameraOffset().z / M2_LEVEL.CameraOffset().length();
		const auto cos_of_player_to_camera_angle =
			sqrtf(1.0f - sin_of_player_to_camera_angle * sin_of_player_to_camera_angle);

		const auto y_offset = (F(screen_center_to_mouse_position_px.y) / m3::Ppm()) * sin_of_player_to_camera_angle;
		const auto z_offset = -(F(screen_center_to_mouse_position_px.y) / m3::Ppm()) * cos_of_player_to_camera_angle;
		const auto x_offset = F(screen_center_to_mouse_position_px.x) / m3::Ppm();
		const auto player_position = m3::FocusPositionM();
		const auto mouse_position_world_m =
			m3::VecF{player_position.x + x_offset, player_position.y + y_offset, player_position.z + z_offset};

		// Create Line from camera to mouse position
		const auto ray_to_mouse = m3::Line::from_points(m3::CameraPositionM(), mouse_position_world_m);
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
		const auto camera_position = _level->objects[_level->cameraId].position;
		_mouse_position_world_m = *_screen_center_to_mouse_position_m + camera_position;
	}
}
