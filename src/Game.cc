#include <m2/Game.h>
#include <m2/Log.h>

#include <Level.pb.h>
#include <SDL2/SDL_image.h>
#include <m2/Error.h>
#include <m2/Object.h>
#include <m2/video/Sprite.h>
#include <m2/String.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/sdl/Detail.h>
#include <m2/sheet_editor/Ui.h>
#include <m2/FileSystem.h>
#include <filesystem>
#include <ranges>
#include "m2/component/Graphic.h"
#include <m2/ui/UiAction.h>
#include <m2/game/Key.h>

m2::Game* m2::Game::_instance{};

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

	const auto minimumWindowDims = GameDimensions::EstimateMinimumWindowDimensions(_proxy.gamePpm, _proxy.defaultGameHeightM);
	if ((window = SDL_CreateWindow(_proxy.gameFriendlyName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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

	_dimensions.emplace(renderer, _proxy.gamePpm, _proxy.gameAspectRatioMul, _proxy.gameAspectRatioDiv);

	SDL_Surface* lightSurface = IMG_Load(_resources.GetRadialWhiteToBlackGradientPath().string().c_str());
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
	if ((font = TTF_OpenFont(_resources.GetDefaultFontPath().string().c_str(), _proxy.default_font_size)) == nullptr) {
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
	spriteEffectsSheet = SpriteEffectsSheet{renderer, _proxy.lightning};

	{
		auto sheets_pb = pb::json_file_to_message<pb::SpriteSheets>(_resources.GetSpriteSheetsPath());
		if (!sheets_pb) {
			throw M2_ERROR(sheets_pb.error());
		}
		spriteSheetsPb = *sheets_pb;
	}
	spriteSheets = SpriteSheet::LoadSpriteSheets(*spriteSheetsPb, renderer, _proxy.lightning);
	_sprites = LoadSprites(spriteSheets, spriteSheetsPb->text_labels(), *spriteEffectsSheet);
	LOG_INFO("Loaded sprites", _sprites.size());

	auto backgroundSprites = _sprites | std::views::filter(IsSpriteBackgroundTile) | std::views::transform(ToSpriteType);
	level_editor_background_sprites = std::vector<m2g::pb::SpriteType>{backgroundSprites.begin(), backgroundSprites.end()};
	LOG_INFO("Loaded level editor background sprites", level_editor_background_sprites.size());

	_objectBlueprints = LoadObjectBlueprints(_proxy.objectBlueprints);

	named_items = pb::LUT<pb::Item, NamedItem>::load(_resources.GetItemsPath(), &pb::Items::items);
	LOG_INFO("Loaded named items", named_items.size());

	animations = pb::LUT<pb::Animation, Animation>::load(_resources.GetAnimationsPath(), &pb::Animations::animations);
	LOG_INFO("Loaded animations", animations.size());

	songs = pb::LUT<pb::Song, Song>::load(_resources.GetSongsPath(), &pb::Songs::songs);
	LOG_INFO("Loaded songs", songs.size());

	keyToScancodeMap = GenerateKeyToScancodeMap(_resources.GetKeysPath());
	scancodeToKeyMap = GenerateScancodeToKeyMap(_resources.GetKeysPath());
	LOG_INFO("Loaded keys and scancodes", keyToScancodeMap.size());
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

m2::void_expected m2::Game::HostTurnBasedGame(unsigned max_connection_count) {
	if (not std::holds_alternative<std::monostate>(_multiPlayerComponents)) {
		throw M2_ERROR("Hosting game requires no other multiplayer threads to exist");
	}

	LOG_INFO("Creating server instance...");
	_multiPlayerComponents.emplace<TurnBasedServerComponents>();
	std::get<TurnBasedServerComponents>(_multiPlayerComponents).serverActorInterface.emplace(max_connection_count);
	LOG_DEBUG("Real ServerThread created");

	// Wait until the server is up
	while (not ServerThread().IsListening()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	// TODO prevent other clients from joining until the host client joins

	LOG_INFO("Server is listening, joining the game as host client...");
	std::get<TurnBasedServerComponents>(_multiPlayerComponents).hostClientThread.emplace();
	LOG_DEBUG("Real TurnBasedHostClientThread created");

	return {};
}
m2::void_expected m2::Game::HostLockstepGame(unsigned max_connection_count) {
	if (not std::holds_alternative<std::monostate>(_multiPlayerComponents)) {
		throw M2_ERROR("Hosting game requires no other multiplayer threads to exist");
	}

	LOG_INFO("Creating server...");
	_multiPlayerComponents.emplace<multiplayer::lockstep::ServerComponents>();
	std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).serverActorInterface.emplace(max_connection_count);
	LOG_DEBUG("Server created");

	// Wait until the lobby is open
	while (not GetLockstepServerActor().IsLobbyOpen()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	// TODO prevent other clients from joining until the host client joins

	LOG_INFO("Server is listening, joining the game as host client...");
	std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).hostClientActorInterface.emplace(network::IpAddressAndPort{
		.ipAddress = network::IpAddress::CreateFromString("127.0.0.1"),
		.port = network::Port::CreateFromHostOrder(1162)
	});
	LOG_DEBUG("Host client created");

	// Wait until the host client connects to the server
	while (GetLockstepHostClientActor().IsSearchingForServer()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

	GetLockstepHostClientActor().SetReadyState(true);

	return {};
}

m2::void_expected m2::Game::JoinTurnBasedGame(const std::string& addr) {
	if (not std::holds_alternative<std::monostate>(_multiPlayerComponents)) {
		throw M2_ERROR("Joining game requires no other multiplayer threads to exist");
	}

	_multiPlayerComponents.emplace<network::TurnBasedRealClientThread>(addr);
	return {};
}
void m2::Game::LeaveGame() {
	_multiPlayerComponents = std::monostate{};
}

bool m2::Game::AddBot() {
	if (not IsServer()) {
		throw M2_ERROR("Only server can add bots");
	}

	// Create an instance at the end of the list
	auto& botList = std::get<TurnBasedServerComponents>(_multiPlayerComponents).botClientThreads;
	const auto it = botList.emplace(botList.end());
	LOG_INFO("Joining the game as bot client...");

	LOG_DEBUG("Destroying temporary TurnBasedBotClientThread...");
	it->~TurnBasedBotClientThread(); // Destruct the default object
	LOG_DEBUG("Temporary TurnBasedBotClientThread destroyed, creating real TurnBasedBotClientThread");

	new (&*it) network::TurnBasedBotClientThread(std::in_place);
	LOG_DEBUG("Real TurnBasedBotClientThread created");

	if (it->is_active()) {
		return true;
	}
	LOG_WARN("TurnBasedBotClientThread failed to connect, destroying client");
	botList.erase(it);
	return false;
}

m2::network::TurnBasedBotClientThread& m2::Game::FindBot(const int receiver_index) {
	if (not IsServer()) {
		throw M2_ERROR("Only server may hold bots");
	}

	auto& botList = std::get<TurnBasedServerComponents>(_multiPlayerComponents).botClientThreads;
	const auto it = std::ranges::find_if(botList, [&](const auto& bot) { return bot.GetReceiverIndex() == receiver_index; });
	if (it == botList.end()) {
		throw M2_ERROR("Bot not found");
	}
	return *it;
}

int m2::Game::TotalPlayerCount() {
	if (IsServer()) {
		return ServerThread().GetClientCount();
	}
	if (IsRealClient()) {
		return TurnBasedRealClientThread().total_player_count();
	}
	throw M2_ERROR("Not a multiplayer game");
}

int m2::Game::SelfIndex() {
	if (IsServer()) {
		return 0;
	}
	if (IsRealClient()) {
		return TurnBasedRealClientThread().self_index();
	}
	throw M2_ERROR("Not a multiplayer game");
}

int m2::Game::TurnHolderIndex() {
	if (IsServer()) {
		return ServerThread().GetTurnHolderIndex();
	}
	if (IsRealClient()) {
		return TurnBasedRealClientThread().turn_holder_index();
	}
	throw M2_ERROR("Not a multiplayer game");
}

bool m2::Game::IsOurTurn() {
	if (IsServer()) {
		return ServerThread().IsOurTurn();
	}
	if (IsRealClient()) {
		if (TurnBasedRealClientThread().is_started()) {
			return TurnBasedRealClientThread().is_our_turn();
		}
		return false;
	}
	throw M2_ERROR("Not a multiplayer game");
}

void m2::Game::QueueClientCommand(const m2g::pb::TurnBasedClientCommand& cmd) {
	if (IsServer()) {
		TurnBasedHostClientThread().queue_client_command(cmd);
	} else if (IsRealClient()) {
		TurnBasedRealClientThread().queue_client_command(cmd);
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}

m2::void_expected m2::Game::LoadSinglePlayer(
	const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	ResetState();
	// Reinit dimensions with proxy in case an editor was initialized before
	_dimensions->SetGameAspectRatio(_proxy.gameAspectRatioMul, _proxy.gameAspectRatioDiv);
	_level.emplace();
	return _level->InitSinglePlayer(level_path_or_blueprint, level_name);
}

m2::void_expected m2::Game::LoadTurnBasedMultiPlayerAsHost(
	const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	ResetState();
	// Reinit dimensions with proxy in case an editor was initialized before
	_dimensions->SetGameAspectRatio(_proxy.gameAspectRatioMul, _proxy.gameAspectRatioDiv);
	_level.emplace();

	auto success = _level->InitTurnBasedMultiPlayerAsHost(level_path_or_blueprint, level_name);
	m2ReflectUnexpected(success);

	// Execute the first server update, which will trigger clients to initialize their levels, but not fully.
	M2_GAME.ServerThread().SendServerUpdate();
	// Manually set the TurnBasedHostClientThread state to STARTED, because it doesn't receive ServerUpdates
	M2_GAME.TurnBasedHostClientThread().start_if_ready();
	// If there are bots, we need to handle the first server update
	LOG_DEBUG("Waiting 1s until the first server update is delivered to bots");
	std::this_thread::sleep_for(std::chrono::seconds(1)); // TODO why? system takes some time to deliver the data to bots, even though they are on the same machine
	auto& botList = std::get<TurnBasedServerComponents>(_multiPlayerComponents).botClientThreads;
	for (auto& bot : botList) {
		if (const auto server_update = bot.pop_server_update(); not server_update) {
			throw M2_ERROR("Bot hasn't received the TurnBasedServerUpdate");
		}
	}

	// Populate level
	M2G_PROXY.multi_player_level_server_populate(level_name, *_level->_lb);

	// Execute second server update, which will fully initialize client levels.
	_lastSentOrReceivedServerUpdateSequenceNo = M2_GAME.ServerThread().SendServerUpdate();
	// Act as if TurnBasedServerUpdate is received on the server-side as well
	LOG_DEBUG("Calling server-side post_server_update...");
	_proxy.post_server_update(*_lastSentOrReceivedServerUpdateSequenceNo, false);

	// If there are bots, we need to consume the second server update as bots are never the first turn holder.
	LOG_DEBUG("Waiting 1s until the second server update is delivered to bots");
	std::this_thread::sleep_for(std::chrono::seconds(1));
	for (auto& bot : botList) {
		if (const auto server_update = bot.pop_server_update(); not server_update) {
			throw M2_ERROR("Bot hasn't received the TurnBasedServerUpdate");
		}
	}

	return success;
}

m2::void_expected m2::Game::LoadTurnBasedMultiPlayerAsGuest(
	const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name) {
	_level.reset();
	ResetState();
	// Reinit dimensions with proxy in case an editor was initialized before
	_dimensions->SetGameAspectRatio(_proxy.gameAspectRatioMul, _proxy.gameAspectRatioDiv);
	_level.emplace();

	auto success = _level->InitTurnBasedMultiPlayerAsGuest(level_path_or_blueprint, level_name);
	m2ReflectUnexpected(success);

	// Consume the initial TurnBasedServerUpdate that triggered the level to be initialized
	auto expect_server_update = M2_GAME.TurnBasedRealClientThread().process_server_update();
	m2ReflectUnexpected(expect_server_update);
	_lastSentOrReceivedServerUpdateSequenceNo = expect_server_update->second;
	m2ReturnUnexpectedUnless(expect_server_update->first == network::ServerUpdateStatus::PROCESSED,
			"Unexpected TurnBasedServerUpdate status");
	return {};
}

m2::void_expected m2::Game::LoadLevelEditor(const std::string& level_resource_path) {
	_level.reset();
	ResetState();
	// Reinit dimensions with default parameters
	_dimensions->SetGameAspectRatio(m2::Proxy{}.gameAspectRatioMul, m2::Proxy{}.gameAspectRatioDiv);
	_level.emplace();
	return _level->InitLevelEditor(level_resource_path);
}

m2::void_expected m2::Game::LoadSheetEditor() {
	_level.reset();
	ResetState();
	// Reinit dimensions with default parameters
	_dimensions->SetGameAspectRatio(m2::Proxy{}.gameAspectRatioMul, m2::Proxy{}.gameAspectRatioDiv);
	_level.emplace();
	return _level->InitSheetEditor(_resources.GetSpriteSheetsPath());
}

m2::void_expected m2::Game::LoadBulkSheetEditor() {
	_level.reset();
	ResetState();
	// Reinit dimensions with default parameters
	_dimensions->SetGameAspectRatio(m2::Proxy{}.gameAspectRatioMul, m2::Proxy{}.gameAspectRatioDiv);
	_level.emplace();
	return _level->InitBulkSheetEditor(_resources.GetSpriteSheetsPath());
}

void m2::Game::ResetState() { events.Clear(); }

void m2::Game::HandleQuitEvent() {
	if (events.PopQuit()) {
		quit = true;
	}
}

void m2::Game::HandleWindowResizeEvent() {
	if (events.PopWindowResize()) {
		OnWindowResize();
	}
}

void m2::Game::HandleConsoleEvent() {
	if (events.PopKeyPress(m2g::pb::KeyType::CONSOLE)) {
		if (UiPanel::create_and_run_blocking(&console_ui).IsQuit()) {
			quit = true;
		}
	}
}

void m2::Game::HandlePauseEvent() {
	if (events.PopKeyPress(m2g::pb::KeyType::PAUSE)) {
		// Select the correct pause menu
		const UiPanelBlueprint* pauseMenuBlueprint{};
		if (std::holds_alternative<splayer::State>(GetLevel().stateVariant)) {
			pauseMenuBlueprint = _proxy.PauseMenuBlueprint();
		} else if (std::holds_alternative<level_editor::State>(GetLevel().stateVariant)) {
			pauseMenuBlueprint = nullptr;
		} else if (std::holds_alternative<sheet_editor::State>(GetLevel().stateVariant)) {
			pauseMenuBlueprint = &sheet_editor_main_menu;
		} else if (std::holds_alternative<bulk_sheet_editor::State>(GetLevel().stateVariant)) {
			pauseMenuBlueprint = nullptr;
		}
		// Execute pause menu if found, exit if QUIT is returned
		if (pauseMenuBlueprint && UiPanel::create_and_run_blocking(pauseMenuBlueprint).IsQuit()) {
			quit = true;
		}
	}
}

void m2::Game::HandleHudEvents() {
	if (_level->_semiBlockingUiPanel) {
		_level->_semiBlockingUiPanel->HandleEvents(events, _level->isPanning)
				.IfQuit([this] {
					quit = true;
				})
				.IfAnyReturn([this](const ReturnBase&) {
					// The return object is discarded. Remove the state.
					_level->_semiBlockingUiPanel.reset();
				});

		// After semi-blocking UI, events are cleared to prevent the game objects from receiving it.
		events.Clear();
		// Handling of events of other UI panels are also skipped.
	} else {
		// Mouse hover UI panel doesn't receive events, but based on the mouse position, it may be moved
		IF(_level->_mouseHoverUiPanel)->SetTopLeftPosition(_level->CalculateMouseHoverUiPanelTopLeftPosition());

		// The order of event handling is the reverse of the drawing order
		for (auto &panel : std::ranges::reverse_view(_level->_customNonblockingUiPanels)) {
			auto action{panel.HandleEvents(events, _level->isPanning)};
			action.IfQuit([this] { quit = true; });
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
	if ((IsServer() && ServerThread().HasBeenShutdown()) || (IsRealClient() && TurnBasedRealClientThread().is_shutdown())) {
		_level.reset();
		ResetState();
		_multiPlayerComponents = std::monostate{};
		// Execute main menu
		if (UiPanel::create_and_run_blocking(_proxy.MainMenuBlueprint()).IsQuit()) {
			quit = true;
		}
	} else if (IsServer()) {
		if (const auto disconnectedClientIndex = ServerThread().PopDisconnectedClientEvent()) {
			// TODO handle
		}
	} else if (IsRealClient()) {
		// Check if the client has reconnected and needs to be set as ready
		if (TurnBasedRealClientThread().is_reconnected()) {
			// Set as ready using the same ready_token
			M2_GAME.TurnBasedRealClientThread().set_ready(true);
			// Expect TurnBasedServerUpdate, handle it normally
		}

		// Check if the client has disconnected
		if (TurnBasedRealClientThread().has_reconnection_timed_out()) {
			_proxy.handle_disconnection_from_server();
			// TODO handle
		}

		// Check if the server has behaved unexpectedly
		if (TurnBasedRealClientThread().is_server_unrecognized()) {
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
		// Check if any of the bots need to handle the TurnBasedServerUpdate
		for (auto& bot : std::get<TurnBasedServerComponents>(_multiPlayerComponents).botClientThreads) {
			if (auto server_update = bot.pop_server_update()) {
				_proxy.bot_handle_server_update(*server_update);
			}
		}

		// Handle client command
		if (const auto client_command = ServerThread().PopCommandFromTurnHolderEvent()) {
			if (const auto new_turn_holder = _proxy.handle_client_command(ServerThread().GetTurnHolderIndex(), client_command->client_command())) {
				if (*new_turn_holder < 0) {
					_server_update_necessary = true;
					_server_update_with_shutdown = true;
				} else {
					ServerThread().SetTurnHolder(*new_turn_holder);
					_server_update_necessary = true;
				}
			}
		}

		// Handle server command
		if (const auto server_command = TurnBasedHostClientThread().pop_server_command()) {
			_proxy.handle_server_command(*server_command);
		}
		// Check if any of the bots need to handle the TurnBasedServerCommand
		for (auto& bot : std::get<TurnBasedServerComponents>(_multiPlayerComponents).botClientThreads) {
			if (auto server_command = bot.pop_server_command()) {
				_proxy.bot_handle_server_command(*server_command, *bot.GetReceiverIndex());
			}
		}
	} else if (IsRealClient()) {
		// Handle server command
		if (const auto server_command = TurnBasedRealClientThread().pop_server_command()) {
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
	// Integrate physics
	for (auto* world : _level->world) {
		if (world) {
			world->Step(phy_period, velocity_iterations, position_iterations);
		}
	}
	// Update positions
	for (auto& phy : _level->physics) {
		for (const auto& body : phy.body) {
			if (body && body->IsEnabled()) {
				auto& obj = phy.Owner();
				auto oldPosition = obj.position;
				// Update object
				obj.position = body->GetPosition();
				obj.orientation = body->GetAngle();
				// Update draw list if necessary
				if (oldPosition != obj.position) {
					const auto gfxId = obj.GetGraphicId();
					const auto poolAndDrawList = _level->GetGraphicPoolAndDrawList(gfxId);
					poolAndDrawList.second->QueueUpdate(phy.OwnerId(), obj.position);
				}
				break;
			}
		}
	}
	// Re-sort draw lists
	for (auto& drawList : _level->fgDrawLists) {
		drawList.Update();
	}
	// If the world is NOT static, the pathfinder's cache should be cleared.
	if (not _proxy.world_is_static) {
		_level->pathfinder->clear_cache();
	}
}

void m2::Game::ExecutePostStep() {
	if (IsServer()) {
		if (_server_update_necessary) {
			LOG_DEBUG("Server update is necessary, sending TurnBasedServerUpdate...");
			_lastSentOrReceivedServerUpdateSequenceNo = ServerThread().SendServerUpdate(_server_update_with_shutdown);
			_server_update_necessary = false; // Unset flag

			// Act as if TurnBasedServerUpdate is received on the server-side as well
			LOG_DEBUG("Calling server-side post_server_update...");
			_proxy.post_server_update(*_lastSentOrReceivedServerUpdateSequenceNo, _server_update_with_shutdown);

			// Shutdown the game if necessary
			if (_server_update_with_shutdown) {
				_server_update_with_shutdown = false;
				// Game will be restarted in handle_network_events
			}
		}
	} else if (IsRealClient()) {
		// Handle TurnBasedServerUpdate
		auto status = TurnBasedRealClientThread().process_server_update();
		m2SucceedOrThrowError(status);

		if (status->first == network::ServerUpdateStatus::PROCESSED || status->first == network::ServerUpdateStatus::PROCESSED_SHUTDOWN) {
			_lastSentOrReceivedServerUpdateSequenceNo = status->second;
			LOG_DEBUG("Calling client-side post_server_update...");
			_proxy.post_server_update(*_lastSentOrReceivedServerUpdateSequenceNo, status->first == network::ServerUpdateStatus::PROCESSED_SHUTDOWN);
		}
		if (status->first == network::ServerUpdateStatus::PROCESSED_SHUTDOWN) {
			TurnBasedRealClientThread().shutdown();
			// Game will be restarted in handle_network_events
		}
	}

	if (not _level->IsEditor()) {
		_proxy.OnPostStep();
		ExecuteDeferredActions();
	}

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
	for (auto& gfx : _level->fgGraphics) {
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
		action.IfQuit([this] { quit = true; });
		if (auto anyReturnContainer = action.ExtractAnyReturnContainer()) {
			// If UI returned, kill the panel. We cannot delete it yet, the iterator is held by the client, but we
			// can replace it with a killed object that can hold the AnyReturnContainer instead.
			panel.KillWithReturnValue(std::move(*anyReturnContainer));
		}
	}
	IF(_level->_mouseHoverUiPanel)->UpdateContents(_delta_time_s);
	if (_level->_semiBlockingUiPanel) {
		_level->_semiBlockingUiPanel->UpdateContents(_delta_time_s)
				.IfQuit([this] {
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

void m2::Game::Draw() {
	// Check if only one background layer needs to be drawn
	const auto onlyBackgroundLayerToDraw = [&]() -> std::optional<BackgroundDrawLayer> {
		if (std::holds_alternative<level_editor::State>(_level->stateVariant)) {
			if (const auto& rightHudName = _level->RightHud()->Name();
					rightHudName == "PaintBgRightHud" || rightHudName == "SampleBgRightHud" || rightHudName == "SelectBgRightHud") {
				const auto& levelEditorState = std::get<level_editor::State>(_level->stateVariant);
				return levelEditorState.GetSelectedBackgroundLayer();
			}
		}
		return std::nullopt;
	}();

	for (const auto& layer : gDrawOrder) {
		// Skip if another background layer needs drawing
		if (onlyBackgroundLayerToDraw && std::holds_alternative<BackgroundDrawLayer>(layer)
				&& *onlyBackgroundLayerToDraw != std::get<BackgroundDrawLayer>(layer)) {
			continue;
		}
		if (const auto poolAndDrawList = _level->GetGraphicPoolAndDrawList(layer); not poolAndDrawList.second) {
			// Draw background
			for (auto& gfx : poolAndDrawList.first) {
				if (gfx.enabled && gfx.draw) {
					IF(gfx.onDraw)(gfx);
				}
			}
		} else {
			// Draw foreground
			for (const auto gfxId : *poolAndDrawList.second) {
				if (auto& gfx = _level->fgGraphics[gfxId]; gfx.enabled && gfx.draw) {
					IF(gfx.onDraw)(gfx);
				}
			}
		}
	}
}

void m2::Game::DrawLights() {
	for (auto& light : _level->lights) {
		IF(light.onDraw)(light);
	}
}

void m2::Game::ExecutePostDraw() {
	for (auto& gfx : _level->fgGraphics) {
		if (gfx.enabled) {
			IF(gfx.postDraw)(gfx);
		}
	}
}

void m2::Game::DebugDraw() {
#ifdef DEBUG
	for (int i = 0; i < I(gPhysicsLayerCount); ++i) {
		if (_level->world[i]) {
			_level->world[i]->DebugDraw();
		}
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
	sdl_rect = static_cast<SDL_Rect>(_dimensions->TopEnvelope());
	SDL_RenderFillRect(renderer, &sdl_rect);
	sdl_rect = static_cast<SDL_Rect>(_dimensions->BottomEnvelope());
	SDL_RenderFillRect(renderer, &sdl_rect);
	sdl_rect = static_cast<SDL_Rect>(_dimensions->LeftEnvelope());
	SDL_RenderFillRect(renderer, &sdl_rect);
	sdl_rect = static_cast<SDL_Rect>(_dimensions->RightEnvelope());
	SDL_RenderFillRect(renderer, &sdl_rect);
}

void m2::Game::FlipBuffers() const { SDL_RenderPresent(renderer); }

void m2::Game::OnWindowResize() {
	_dimensions->OnWindowResize();
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
		for (auto& gfx : _level->fgGraphics) {
			gfx.textLabelRect = {};
		}
		for (auto& terrainGraphics : std::ranges::reverse_view(_level->bgGraphics)) {
			for (auto& gfx : terrainGraphics) {
				gfx.textLabelRect = {};
			}
		}
	}
}
void m2::Game::SetScale(const float scale) {
	_dimensions->SetScale(scale);
	if (_level) {
		// Clear text label rectangles so that they are regenerated with new size
		for (auto& gfx : _level->fgGraphics) {
			gfx.textLabelRect = {};
		}
		for (auto& terrainGraphics : std::ranges::reverse_view(_level->bgGraphics)) {
			for (auto& gfx : terrainGraphics) {
				gfx.textLabelRect = {};
			}
		}
	}
}
void m2::Game::SetGameHeightM(const float heightM) {
	_dimensions->SetGameHeightM(heightM);
	if (_level) {
		// Clear text label rectangles so that they are regenerated with new size
		for (auto& gfx : _level->fgGraphics) {
			gfx.textLabelRect = {};
		}
		for (auto& terrainGraphics : std::ranges::reverse_view(_level->bgGraphics)) {
			for (auto& gfx : terrainGraphics) {
				gfx.textLabelRect = {};
			}
		}
	}
}

void m2::Game::ForEachSprite(const std::function<bool(m2g::pb::SpriteType, const Sprite&)>& op) const {
	for (int i = 0; i < pb::enum_value_count<m2g::pb::SpriteType>(); ++i) {
		const auto type = pb::enum_value<m2g::pb::SpriteType>(i);
		if (const auto& spriteOrTextLabel = GetSpriteOrTextLabel(type);
				std::holds_alternative<Sprite>(spriteOrTextLabel) && not op(type, std::get<Sprite>(spriteOrTextLabel))) {
			return;
		}
	}
}

void m2::Game::ForEachNamedItem(const std::function<bool(m2g::pb::ItemType, const NamedItem&)>& op) const {
	for (int i = 0; i < pb::enum_value_count<m2g::pb::ItemType>(); ++i) {
		if (const auto type = pb::enum_value<m2g::pb::ItemType>(i); not op(type, GetNamedItem(type))) {
			return;
		}
	}
}
std::optional<m2g::pb::SpriteType> m2::Game::GetMainSpriteOfObject(const m2g::pb::ObjectType ot) const {
	const auto objectTypeIndex = pb::enum_index(ot);
	const auto defaultSpriteType = _objectBlueprints[objectTypeIndex].defaultSpriteType;
	if (not defaultSpriteType) {
		return std::nullopt;
	}
	return defaultSpriteType;
}
void m2::Game::ForEachObjectWithMainSprite(const std::function<bool(m2g::pb::ObjectType, m2g::pb::SpriteType)>& op) const {
	for (const auto& objectBlueprint : _objectBlueprints) {
		if (objectBlueprint.defaultSpriteType) {
			if (not op(objectBlueprint.objectType, objectBlueprint.defaultSpriteType)) {
				return;
			}
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

m2::sdl::TextureUniquePtr m2::Game::DrawGameToTexture(const VecF& camera_position) {
	// Temporarily change camera position
	const auto prev_camera_position = GetLevel().Camera()->position;
	GetLevel().Camera()->position = camera_position;

	// Create an empty render target
	auto render_target = sdl::create_drawable_texture_of_screen_size();
	// Temporarily change render target
	const auto prev_render_target = SDL_GetRenderTarget(renderer);
	SDL_SetRenderTarget(renderer, render_target.get());

	// Draw
	ClearBackBuffer();
	Draw();
	DrawLights();
	DrawEnvelopes();

	// Reinstate old render target
	SDL_SetRenderTarget(renderer, prev_render_target);

	// Reinstate old camera position
	GetLevel().Camera()->position = prev_camera_position;

	return render_target;
}
bool m2::Game::IsMouseOnAnyUiPanel() const {
	// If semi-blocking UI panel is action, we act as if the UI panel covers the whole screen
	if (_level->_semiBlockingUiPanel) {
		return true;
	}
	// Otherwise, check if the mouse is on top of the other UI panels known to Level
	const auto mouse_position = events.MousePosition();
	if (_level->_leftHudUiPanel && _level->_leftHudUiPanel->Rect().DoesContain(mouse_position)) {
		return true;
	}
	if (_level->_rightHudUiPanel && _level->_rightHudUiPanel->Rect().DoesContain(mouse_position)) {
		return true;
	}
	if (_level->_messageBoxUiPanel && _level->_messageBoxUiPanel->Rect().DoesContain(mouse_position)) {
		return true;
	}
	for (auto &panel : _level->_customNonblockingUiPanels) {
		if (panel.Rect().DoesContain(mouse_position)) {
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
			const auto& sound_position = sound_emitter.Owner().position;
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

void m2::Game::AddDeferredAction(const std::function<void()>& action) {
	_level->deferredActions.push(action);
}

void m2::Game::ExecuteDeferredActions() {
	// Execute deferred actions one by one. A deferred action may insert another deferred action into the queue.
	// Thus, we cannot iterate over the queue, we must pop one by one.
	while (not _level->deferredActions.empty()) {
		_level->deferredActions.front()();
		_level->deferredActions.pop();
	}
}

void m2::Game::RecalculateMousePosition() const {
	const auto mouse_position = events.MousePosition();
	const auto screen_center_to_mouse_position_px =
		VecI{mouse_position.x - Dimensions().WindowDimensions().x / 2, mouse_position.y - Dimensions().WindowDimensions().y / 2};
	_screen_center_to_mouse_position_m = VecF{
		F(screen_center_to_mouse_position_px.x) / Dimensions().OutputPixelsPerMeter(), F(screen_center_to_mouse_position_px.y) / Dimensions().OutputPixelsPerMeter()};

	if (IsProjectionTypePerspective(_level->ProjectionType())) {
		// Mouse moves on the plane centered at the player looking towards the camera
		// Find m3::VecF of the mouse position in the world starting from the player position
		const auto sin_of_player_to_camera_angle = M2_LEVEL.CameraOffset().z / M2_LEVEL.CameraOffset().length();
		const auto cos_of_player_to_camera_angle =
			sqrtf(1.0f - sin_of_player_to_camera_angle * sin_of_player_to_camera_angle);

		const auto y_offset = F(screen_center_to_mouse_position_px.y) / m3::Ppm() * sin_of_player_to_camera_angle;
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
