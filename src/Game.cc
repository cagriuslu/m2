#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/Error.h>
#include <m2/Object.h>
#include <m2/video/Sprite.h>
#include <m2/bulksheeteditor/Ui.h>
#include <m2/sdl/Detail.h>
#include <m2/sheeteditor/Ui.h>
#include <m2/component/Graphic.h>
#include <m2/ui/UiAction.h>
#include <m2/game/Key.h>
#include <m2/String.h>
#include <M2.orm.h>
#include <Level.pb.h>
#include <CMakeProject.h>
#include <SDL2/SDL_image.h>
#include <genORM/genORM.h>
#include <filesystem>
#include <ranges>

using namespace m2;

namespace {
	constexpr auto MAX_LOCKSTEP_CONNECTION_COUNT = 4;

	SDL_Window* CreateWindow(const int gamePpm, const float defaultGameHeightM, const char* gameFriendlyName) {
		const auto minimumWindowDims = GameDimensions::EstimateMinimumWindowDimensions(gamePpm, defaultGameHeightM);
		if (auto* window = SDL_CreateWindow(gameFriendlyName, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				minimumWindowDims.x, minimumWindowDims.y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) {
			SDL_SetWindowMinimumSize(window, minimumWindowDims.x, minimumWindowDims.y);
			SDL_StopTextInput(); // Text input begins activated (sometimes)
			return window;
		}
		throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
	}

	SDL_Cursor* CreateCursor() {
		auto* cursor = SdlUtils_CreateCursor();
		SDL_SetCursor(cursor);
		return cursor;
	}

	uint32_t GetWindowPixelFormat(SDL_Window* const window) {
		if (const auto pixel_format = SDL_GetWindowPixelFormat(window); pixel_format == SDL_PIXELFORMAT_UNKNOWN) {
			throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
		} else {
			return pixel_format;
		}
	}

	TTF_Font* OpenFont(const char* path, const int fontSize) {
		if (auto* font = TTF_OpenFont(path, fontSize)) {
			return font;
		}
		throw M2_ERROR("TTF error: " + std::string{TTF_GetError()});
	}
}

Game* Game::_instance{};

void Game::InitSystems() {
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
}
void Game::CreateInstance() {
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

void Game::DestroyInstance() {
	DEBUG_FN();
	delete _instance;
	_instance = nullptr;
}
void Game::DeinitSystems() {
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

Game::Game() : window(CreateWindow(_proxy.gamePpm, _proxy.defaultGameHeightM, _proxy.gameFriendlyName.c_str())),
		cursor(CreateCursor()), pixel_format(GetWindowPixelFormat(window)),
		font(OpenFont(_resources.GetDefaultFontPath().c_str(), _proxy.default_font_size)),
		systemFont(OpenFont(_resources.GetSystemFontPath().c_str(), systemFontSize)) {
	// ReSharper disable once CppDFAConstantConditions
	if (_proxy.areGraphicsPixelated) {
		// ReSharper disable once CppDFAUnreachableCode
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	} else {
		// ReSharper disable once CppDFAUnreachableCode
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

	const pb::Cards* inMemoryCards = _proxy.cards ? &*_proxy.cards : nullptr;
	cards = pb::MessageLUT<pb::Card, Card>::load(inMemoryCards, _resources.GetCardsPath(), &pb::Cards::cards);
	LOG_INFO("Loaded named cards", cards.size());

	animations = pb::MessageLUT<pb::Animation, Animation>::load(static_cast<pb::Animations*>(nullptr), _resources.GetAnimationsPath(), &pb::Animations::animations);
	LOG_INFO("Loaded animations", animations.size());

	songs = pb::MessageLUT<pb::Song, Song>::load(static_cast<pb::Songs*>(nullptr), _resources.GetSongsPath(), &pb::Songs::songs);
	LOG_INFO("Loaded songs", songs.size());

	keyToScancodeMap = GenerateKeyToScancodeMap(_resources.GetKeysPath());
	scancodeToKeyMap = GenerateScancodeToKeyMap(_resources.GetKeysPath());
	LOG_INFO("Loaded keys and scancodes", keyToScancodeMap.size());
}

Game::~Game() {
	_level.reset();
	audio_manager.reset();
	SDL_DestroyRenderer(renderer);
	SDL_FreeCursor(cursor);
	SDL_DestroyWindow(window);
}

void_expected Game::HostTurnBasedGame(unsigned max_connection_count) {
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
	std::get<TurnBasedServerComponents>(_multiPlayerComponents).hostClientThread.emplace(std::in_place);
	LOG_DEBUG("Real TurnBasedHostClientThread created");

	return {};
}
void_expected Game::HostLockstepGame(unsigned max_connection_count, const network::IpAddress& multicastInterface) {
	if (MAX_LOCKSTEP_CONNECTION_COUNT < max_connection_count) {
		return make_unexpected("Given max connection count is higher than the limit: " + ToString(max_connection_count));
	}
	if (not std::holds_alternative<std::monostate>(_multiPlayerComponents)) {
		throw M2_ERROR("Hosting game requires no other multiplayer threads to exist");
	}

	_multiPlayerComponents.emplace<multiplayer::lockstep::ServerComponents>();
	LOG_DEBUG("Starting network discovery...");
	std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).networkDiscoveryActorInterface.emplace(network::discovery::NetworkDiscoveryActor::SpeakerParameters{
		.multicastAddress = network::IpAddressAndPort{
			.ipAddress = network::IpAddress::CreateFromString(options::GetMulticastDiscoveryAddress()),
			.port = network::Port::CreateFromHostOrder(options::GetMulticastDiscoveryPort())
		},
		.multicastInterface = multicastInterface,
		.gameHash = Hash(),
		.gamePort = network::Port::CreateFromHostOrder(options::GetPort())
	});
	LOG_INFO("Network discovery started");

	LOG_DEBUG("Creating server...");
	std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).serverActorInterface.emplace(max_connection_count);
	LOG_INFO("Server created");

	// Wait until the lobby is open
	while (not GetLockstepServerActor().IsLobbyOpen()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	// TODO prevent other clients from joining until the host client joins

	LOG_INFO("Server is listening, joining the game as host client...");
	std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).hostClientActorInterface.emplace(network::IpAddressAndPort{
		.ipAddress = network::IpAddress::CreateFromString("127.0.0.1"),
		.port = network::Port::CreateFromHostOrder(options::GetPort())
	});
	LOG_DEBUG("Host client created");

	// Wait until the host client connects to the server
	while (GetLockstepHostClientActor().IsSearchingForServer()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

	GetLockstepHostClientActor().SetReadyState(true);

	return {};
}
void_expected Game::EnableLevelSaver(const std::string& fpath) {
	if (std::holds_alternative<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents)) {
		return _level->EmplaceLevelSaver(std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).levelSaverInterface, fpath);
	}
	if (std::holds_alternative<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents)) {
		return _level->EmplaceLevelSaver(std::get<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents).levelSaverInterface, fpath);
	}
	if (std::holds_alternative<multiplayer::lockstep::ReplayComponents>(_multiPlayerComponents)) {
		LOG_WARN("Ignoring attempt to enable level saver while replaying another save");
		return {};
	}
	throw M2_ERROR("Only lockstep level can be saved");
}
void_expected Game::JoinTurnBasedGame(const std::string& addr) {
	if (not std::holds_alternative<std::monostate>(_multiPlayerComponents)) {
		throw M2_ERROR("Joining game requires no other multiplayer threads to exist");
	}

	_multiPlayerComponents.emplace<network::TurnBasedRealClientThread>(addr);
	return {};
}
void_expected Game::JoinLockstepGame(const network::IpAddressAndPort& addr) {
	if (not std::holds_alternative<std::monostate>(_multiPlayerComponents)) {
		throw M2_ERROR("Joining game requires no other multiplayer threads to exist");
	}
	_multiPlayerComponents.emplace<multiplayer::lockstep::ClientComponents>(addr);
	return {};
}
void Game::EnableServerDiscovery() {
	LOG_DEBUG("Starting server discovery...");
	networkDiscoveryActorInterface.emplace(network::discovery::NetworkDiscoveryActor::ListenerParameters{
		.multicastAddress = network::IpAddressAndPort{
			.ipAddress = network::IpAddress::CreateFromString(options::GetMulticastDiscoveryAddress()),
			.port = network::Port::CreateFromHostOrder(options::GetMulticastDiscoveryPort())
		},
		.gameHash = Hash(),
	});
	LOG_INFO("Server discovery started");
}
void Game::DisableServerDiscovery() {
	networkDiscoveryActorInterface.reset();
}
void Game::LeaveGame() {
	_multiPlayerComponents = std::monostate{};
}
bool Game::AddBot() {
	if (not IsTurnBasedServer()) {
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
network::TurnBasedBotClientThread& Game::FindBot(const int receiver_index) {
	if (not IsTurnBasedServer()) {
		throw M2_ERROR("Only server may hold bots");
	}

	auto& botList = std::get<TurnBasedServerComponents>(_multiPlayerComponents).botClientThreads;
	const auto it = std::ranges::find_if(botList, [&](const auto& bot) { return bot.GetReceiverIndex() == receiver_index; });
	if (it == botList.end()) {
		throw M2_ERROR("Bot not found");
	}
	return *it;
}
multiplayer::lockstep::ClientActorInterface& Game::GetLockstepClientActor() {
	if (std::holds_alternative<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents)) {
		return *std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).hostClientActorInterface;
	}
	if (std::holds_alternative<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents)) {
		return std::get<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents).guestClientActorInterface;
	}
	throw M2_ERROR("Not a lockstep multiplayer game");
}
network::discovery::NetworkDiscoveryActorInterface& Game::GetNetworkDiscoveryActorInterface() {
	return *networkDiscoveryActorInterface;
}
multiplayer::lockstep::LevelSaverInterface* Game::GetLockstepLevelSaverInterface() {
	if (std::holds_alternative<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents)) {
		auto& levelSaverInterface = std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).levelSaverInterface;
		return levelSaverInterface ? &*levelSaverInterface : nullptr;
	}
	if (std::holds_alternative<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents)) {
		auto& levelSaverInterface = std::get<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents).levelSaverInterface;
		return levelSaverInterface ? &*levelSaverInterface : nullptr;
	}
	return nullptr;
}
int Game::GetTotalPlayerCount() {
	if (IsTurnBasedServer()) {
		return ServerThread().GetClientCount();
	}
	if (IsRealTurnBasedClient()) {
		return TurnBasedRealClientThread().total_player_count();
	}
	if (std::holds_alternative<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents)) {
		if (const auto& hostClientActor = std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).hostClientActorInterface) {
			return hostClientActor->GetTotalPlayerCount();
		}
	}
	if (std::holds_alternative<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents)) {
		const auto& guestClientActor = std::get<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents).guestClientActorInterface;
		return guestClientActor.GetTotalPlayerCount();
	}
	if (std::holds_alternative<multiplayer::lockstep::ReplayComponents>(_multiPlayerComponents)) {
		return std::get<multiplayer::lockstep::ReplayComponents>(_multiPlayerComponents).playerCount;
	}
	throw M2_ERROR("Not a multiplayer game");
}
int Game::GetSelfIndex() {
	if (IsTurnBasedServer()) {
		return 0;
	}
	if (IsRealTurnBasedClient()) {
		return TurnBasedRealClientThread().self_index();
	}
	if (std::holds_alternative<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents)) {
		if (const auto& hostClientActor = std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).hostClientActorInterface) {
			if (const auto selfIndex = hostClientActor->GetSelfIndex()) {
				return *selfIndex;
			}
		}
	}
	if (std::holds_alternative<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents)) {
		const auto& guestClientActor = std::get<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents).guestClientActorInterface;
		if (const auto selfIndex = guestClientActor.GetSelfIndex()) {
			return *selfIndex;
		}
	}
	if (std::holds_alternative<multiplayer::lockstep::ReplayComponents>(_multiPlayerComponents)) {
		return std::get<multiplayer::lockstep::ReplayComponents>(_multiPlayerComponents).selfIndex;
	}
	throw M2_ERROR("Not a multiplayer game or index isn't known yet");
}
int Game::GetTurnBasedTurnHolderIndex() {
	if (IsTurnBasedServer()) {
		return ServerThread().GetTurnHolderIndex();
	}
	if (IsRealTurnBasedClient()) {
		return TurnBasedRealClientThread().turn_holder_index();
	}
	throw M2_ERROR("Not a multiplayer game");
}
bool Game::IsOurTurn() {
	if (IsTurnBasedServer()) {
		return ServerThread().IsOurTurn();
	}
	if (IsRealTurnBasedClient()) {
		if (TurnBasedRealClientThread().is_started()) {
			return TurnBasedRealClientThread().is_our_turn();
		}
		return false;
	}
	throw M2_ERROR("Not a multiplayer game");
}
void Game::QueueClientCommand(const m2g::pb::TurnBasedClientCommand& cmd) {
	if (IsTurnBasedServer()) {
		TurnBasedHostClientThread().queue_client_command(cmd);
	} else if (IsRealTurnBasedClient()) {
		TurnBasedRealClientThread().queue_client_command(cmd);
	} else {
		throw M2_ERROR("Not a multiplayer game");
	}
}
bool Game::QueuePlayerInput(m2g::pb::LockstepPlayerInput&& playerInput) {
	return GetLockstepClientActor().TryQueueInput(std::move(playerInput));
}

void_expected Game::LoadSinglePlayer(
	const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& level_name) {
	_level.reset();
	ResetState();
	// Reinit dimensions with proxy in case an editor was initialized before
	_dimensions->SetGameAspectRatio(_proxy.gameAspectRatioMul, _proxy.gameAspectRatioDiv);
	_level.emplace();
	return _level->InitSinglePlayer(levelPathOrBlueprint, level_name);
}
void_expected Game::LoadTurnBasedMultiPlayerAsHost(
	const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& level_name) {
	_level.reset();
	ResetState();
	// Reinit dimensions with proxy in case an editor was initialized before
	_dimensions->SetGameAspectRatio(_proxy.gameAspectRatioMul, _proxy.gameAspectRatioDiv);
	_level.emplace();

	auto success = _level->InitTurnBasedMultiPlayerAsHost(levelPathOrBlueprint, level_name);
	m2ReflectUnexpected(success);

	// Execute the first server update, which will trigger clients to initialize their levels, but not fully.
	M2_GAME.ServerThread().SendServerUpdate();
	// Manually set the TurnBasedHostClientThread state to STARTED, because it doesn't receive ServerUpdates
	M2_GAME.TurnBasedHostClientThread().start_if_ready();
	// If there are bots, we need to handle the first server update
	LOG_DEBUG("Waiting 1s until the first server update is delivered to bots");
	std::this_thread::sleep_for(std::chrono::seconds(1));
	auto& botList = std::get<TurnBasedServerComponents>(_multiPlayerComponents).botClientThreads;
	for (auto& bot : botList) {
		if (const auto server_update = bot.pop_server_update(); not server_update) {
			throw M2_ERROR("Bot hasn't received the TurnBasedServerUpdate");
		}
	}

	// Populate level
	M2G_PROXY.turnBasedServerPopulate(level_name, *_level->_lb);

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
void_expected Game::LoadTurnBasedMultiPlayerAsGuest(
	const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& level_name) {
	_level.reset();
	ResetState();
	// Reinit dimensions with proxy in case an editor was initialized before
	_dimensions->SetGameAspectRatio(_proxy.gameAspectRatioMul, _proxy.gameAspectRatioDiv);
	_level.emplace();

	auto success = _level->InitTurnBasedMultiPlayerAsGuest(levelPathOrBlueprint, level_name);
	m2ReflectUnexpected(success);

	// Consume the initial TurnBasedServerUpdate that triggered the level to be initialized
	auto expect_server_update = M2_GAME.TurnBasedRealClientThread().process_server_update();
	m2ReflectUnexpected(expect_server_update);
	_lastSentOrReceivedServerUpdateSequenceNo = expect_server_update->second;
	m2ReturnUnexpectedUnless(expect_server_update->first == network::ServerUpdateStatus::PROCESSED, "Unexpected TurnBasedServerUpdate status");
	return {};
}
void_expected Game::LoadLockstep(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& levelName, const m2g::pb::LockstepGameInitParams& gameInitParams) {
	_level.reset();
	ResetState();
	// Reinit dimensions with proxy in case an editor was initialized before
	_dimensions->SetGameAspectRatio(_proxy.gameAspectRatioMul, _proxy.gameAspectRatioDiv);
	_level.emplace();

	if (std::holds_alternative<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents)) {
		std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).networkDiscoveryActorInterface.reset();
	}

	// Make sure the client state is proper. Game should only be loaded when the lobby is frozen. This is a concern only
	// for the host since the client won't even have access to game init parameters unless the lobby is frozen.
	if (not GetLockstepClientActor().IsLobbyFrozen()) {
		throw M2_ERROR("Unexpected lockstep client actor state");
	}
	LOG_INFO("Loading lockstep level...");

	auto success = _level->InitLockstepMultiPlayer(levelPathOrBlueprint, levelName, gameInitParams);
	m2ReflectUnexpected(success);

	// Send a set of empty player inputs with timecode 0 to signal the beginning of the simulation. This will also help clients synchronize.
	GetLockstepClientActor().StartInputStreaming();

	return {};
}
void_expected Game::ReplayLockstep(const std::string& fpath) {
	auto expectDb = genORM::database::open(fpath.c_str());
	m2ReflectUnexpected(expectDb);
	const auto expectMetadata = orm::LockstepGameMetadata::find_by_rowid(*expectDb, 1);
	m2ReflectUnexpected(expectMetadata);
	if (not expectMetadata.value().has_value()) {
		return make_unexpected("Save file doesn't contain game metadata");
	}
	if (const auto saveGitCommitHash = ToString(expectMetadata.value()->get_git_short_commit_hash()); saveGitCommitHash != GIT_SHORT_COMMIT_HASH) {
		LOG_WARN("Git short commit hash of save file doesn't match with self", saveGitCommitHash);
	}
	const auto playerCount = expectMetadata.value()->get_player_count();
	if (MAX_LOCKSTEP_CONNECTION_COUNT < playerCount) {
		return make_unexpected("Save file contains a game with more players than supported: " + ToString(playerCount));
	}
	const auto selfIndex = expectMetadata.value()->get_self_index();
	if (selfIndex < 0 || playerCount <= selfIndex) {
		return make_unexpected("Save file contains an out of bounds self index: " + ToString(selfIndex));
	}
	pb::Level level;
	{
		if (const auto& levelBlueprintBytes = expectMetadata.value()->get_level_blueprint(); not levelBlueprintBytes.empty()) {
			if (not level.ParseFromArray(levelBlueprintBytes.data(), levelBlueprintBytes.size())) {
				return make_unexpected("Unable to parse the level blueprint inside the save file");
			}
		}
	}
	m2g::pb::LockstepGameInitParams gameInitParams;
	{
		if (const auto& gameInitParamsBytes = expectMetadata.value()->get_game_init_params(); not gameInitParamsBytes.empty()) {
			if (not gameInitParams.ParseFromArray(gameInitParamsBytes.data(), gameInitParamsBytes.size())) {
				return make_unexpected("Unable to parse the lockstep game initialization parameters inside the save file");
			}
		}
	}
	auto name = ToString(expectMetadata.value()->get_level_name());

	if (not std::holds_alternative<std::monostate>(_multiPlayerComponents)) {
		throw M2_ERROR("Replaying a lockstep game requires no other multiplayer threads to exist");
	}
	_multiPlayerComponents.emplace<multiplayer::lockstep::ReplayComponents>(playerCount, selfIndex, multiplayer::lockstep::LevelReplayer{std::move(*expectDb), playerCount});

	_level.reset();
	ResetState();
	// Reinit dimensions with proxy in case an editor was initialized before
	_dimensions->SetGameAspectRatio(_proxy.gameAspectRatioMul, _proxy.gameAspectRatioDiv);
	_level.emplace();
	return _level->InitLockstepMultiPlayer(level, name, gameInitParams);
}
void_expected Game::LoadLevelEditor(const std::string& level_resource_path) {
	_level.reset();
	ResetState();
	// Reinit dimensions with default parameters
	_dimensions->SetGameAspectRatio(m2::Proxy{}.gameAspectRatioMul, m2::Proxy{}.gameAspectRatioDiv);
	_level.emplace();
	return _level->InitLevelEditor(level_resource_path);
}
void_expected Game::LoadSheetEditor() {
	_level.reset();
	ResetState();
	// Reinit dimensions with default parameters
	_dimensions->SetGameAspectRatio(m2::Proxy{}.gameAspectRatioMul, m2::Proxy{}.gameAspectRatioDiv);
	_level.emplace();
	return _level->InitSheetEditor(_resources.GetSpriteSheetsPath());
}
void_expected Game::LoadBulkSheetEditor() {
	_level.reset();
	ResetState();
	// Reinit dimensions with default parameters
	_dimensions->SetGameAspectRatio(m2::Proxy{}.gameAspectRatioMul, m2::Proxy{}.gameAspectRatioDiv);
	_level.emplace();
	return _level->InitBulkSheetEditor(_resources.GetSpriteSheetsPath());
}

void Game::ResetState() { events.Clear(); }

void Game::StartHandlingEvents() {
	if (_eventsAreBeingHandled) {
		throw M2_ERROR("Events were already being handled");
	}
	_eventsAreBeingHandled = true;
}
void Game::HandleQuitEvent() {
	if (events.PopQuit()) {
		quit = true;
	}
}
void Game::HandleWindowResizeEvent() {
	if (events.PopWindowResize()) {
		OnWindowResize();
	}
}
void Game::ExecuteQueuedCommands() {
	while (not _queuedCommands.empty()) {
		if (const auto result = ExecuteCommand(_queuedCommands.front()); std::holds_alternative<UnknownCommand>(result)) {
			LOG_ERROR("Unknown command", _queuedCommands.front());
		} else if (std::holds_alternative<CommandFail>(result)) {
			LOG_ERROR("Command failed with error", std::string{"'"} + _queuedCommands.front() + "'", std::get<CommandFail>(result).error);
		}
		_queuedCommands.pop_front();
	}
}
void Game::HandleConsoleEvent() {
	if (events.PopKeyPress(m2g::pb::KeyType::CONSOLE)) {
		if (UiPanel::create_and_run_blocking(&console_ui).IsQuit()) {
			quit = true;
		}
	}
}
void Game::HandlePauseEvent() {
	if (events.PopKeyPress(m2g::pb::KeyType::PAUSE)) {
		// Select the correct pause menu
		const UiPanelBlueprint* pauseMenuBlueprint{};
		if (std::holds_alternative<m2g::Proxy::LevelState>(GetLevel().stateVariant)) {
			pauseMenuBlueprint = _proxy.PauseMenuBlueprint();
		} else if (std::holds_alternative<leveleditor::State>(GetLevel().stateVariant)) {
			pauseMenuBlueprint = nullptr;
		} else if (std::holds_alternative<sheeteditor::State>(GetLevel().stateVariant)) {
			pauseMenuBlueprint = &sheet_editor_main_menu;
		} else if (std::holds_alternative<bulksheeteditor::State>(GetLevel().stateVariant)) {
			pauseMenuBlueprint = nullptr;
		} else {
			throw M2_ERROR("Implementation error");
		}
		// Execute pause menu if found, exit if QUIT is returned
		if (pauseMenuBlueprint && UiPanel::create_and_run_blocking(pauseMenuBlueprint).IsQuit()) {
			quit = true;
		}
	}
}
void Game::HandleHudEvents() {
	if (_level->_semiBlockingUiPanel) {
		_level->_semiBlockingUiPanel->HandleEvents(events, _level->IsPanning())
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
			auto action{panel.HandleEvents(events, _level->IsPanning())};
			action.IfQuit([this] { quit = true; });
			if (auto anyReturnContainer = action.ExtractAnyReturnContainer()) {
				// If UI returned, kill the panel. We cannot delete it yet, the iterator is held by the client, but we
				// can replace it with a killed object that can hold the AnyReturnContainer instead.
				panel.KillWithReturnValue(std::move(*anyReturnContainer));
			}
		}
		// Remove panels that have returned and has auto-clean property
		std::erase_if(_level->_customNonblockingUiPanels, [](const auto& panel) {
			return panel.IsKilled() && panel.IsAutoClean();
		});
		IF(_level->_messageBoxUiPanel)->HandleEvents(events, _level->IsPanning());
		IF(_level->_rightHudUiPanel)->HandleEvents(events, _level->IsPanning());
		IF(_level->_leftHudUiPanel)->HandleEvents(events, _level->IsPanning());
	}
}
void Game::HandleNetworkEvents() {
	// Check if the game ended
	if ((IsTurnBasedServer() && ServerThread().HasBeenShutdown())
			|| (IsRealTurnBasedClient() && TurnBasedRealClientThread().is_shutdown())
			|| (std::holds_alternative<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents)
				&& not std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).serverActorInterface->IsActorRunning()
				&& not std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).hostClientActorInterface->IsActorRunning())
			|| (std::holds_alternative<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents)
				&& not std::get<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents).guestClientActorInterface.IsActorRunning())
			|| (std::holds_alternative<multiplayer::lockstep::ReplayComponents>(_multiPlayerComponents)
				&& std::get<multiplayer::lockstep::ReplayComponents>(_multiPlayerComponents).levelReplayer.IsFinished())) {
		_level.reset();
		ResetState();
		_multiPlayerComponents = std::monostate{};
		// Execute main menu
		if (UiPanel::create_and_run_blocking(_proxy.MainMenuBlueprint()).IsQuit()) {
			quit = true;
		}
	} else if (IsTurnBasedServer()) {
		if (const auto disconnectedClientIndex = ServerThread().PopDisconnectedClientEvent()) {
			// TODO handle
		}
	} else if (IsRealTurnBasedClient()) {
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
void Game::StopHandlingEvents() {
	if (not _eventsAreBeingHandled) {
		throw M2_ERROR("Events weren't already being handled");
	}
	_eventsAreBeingHandled = false;
}
bool Game::ShouldSimulatePhysics() {
	if (std::holds_alternative<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents)
			|| std::holds_alternative<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents)) {
		return std::holds_alternative<multiplayer::lockstep::ClientActorInterface::SimulatePhysics>(GetLockstepClientActor().SwapInputsIfTimeHasCome());
	}
	return true;
}
void Game::ExecutePreStep(const Stopwatch::Duration& delta) {
	_proxy.OnPreStep(delta);
	ExecuteDeferredActions();
	for (auto& phy : _level->physics) {
		IF(phy.preStep)(phy, delta);
	}
}
void Game::UpdateCharacters(const Stopwatch::Duration& delta) {
	_level->GetCharacterStorage().UpdateCharacters(delta);
}
void Game::ExecuteStep(const Stopwatch::Duration& delta) {
	if (IsTurnBasedServer()) {
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
	} else if (IsRealTurnBasedClient()) {
		// Handle server command
		if (const auto server_command = TurnBasedRealClientThread().pop_server_command()) {
			_proxy.handle_server_command(*server_command);
		}
	} else if (std::holds_alternative<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents)
			|| std::holds_alternative<multiplayer::lockstep::ClientComponents>(_multiPlayerComponents)) {
		if (auto simulationInputs = GetLockstepClientActor().PopSimulationInputs()) {
			if (auto* levelSaverInterface = GetLockstepLevelSaverInterface(); levelSaverInterface) {
				if (not levelSaverInterface->IsActorRunning()) {
					throw M2_ERROR("Level saver stopped running prematurely");
				}
				if (std::ranges::any_of(simulationInputs->allInputsToSimulate, [](const auto& playerInputs) { return not playerInputs.empty(); })) {
					levelSaverInterface->StorePlayerInputs(simulationInputs->timecode, simulationInputs->allInputsToSimulate);
				}
			}
			LOG_NETWORK("Simulating inputs from all players for timecode", simulationInputs->timecode);
			_proxy.HandleLockstepPlayerInputs(simulationInputs->allInputsToSimulate);
			// Calculate game state hash if enough time has passed
			if (0 < simulationInputs->timecode && (simulationInputs->timecode % multiplayer::lockstep::ConnectionToServer::GAME_STATE_REPORT_PERIOD_IN_TICKS) == 0) {
				const auto gameStateHash = [&]() -> int32_t {
					if constexpr (BUILD_IS_DEBUG) {
						if (auto* levelSaverInterface = GetLockstepLevelSaverInterface(); levelSaverInterface) {
							auto debugStateReport = _level->CalculateLockstepDebugStateReport(I(simulationInputs->timecode));
							const auto retval = debugStateReport.game_state_hash();
							levelSaverInterface->StoreDebugStateReport(simulationInputs->timecode, std::move(debugStateReport));
							return retval;
						}
					}
					return _level->CalculateLockstepGameStateHash(I(simulationInputs->timecode));
				}();
				LOG_NETWORK("Game state hash for timecode", simulationInputs->timecode, gameStateHash);
				// This hash will be used during the next report. There should be enough time for the actors to receive and retain them.
				GetLockstepClientActor().StoreGameStateHash(simulationInputs->timecode, gameStateHash);
				// Report the game state hash to server as well. That'll become the ground truth.
				if (std::holds_alternative<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents)) {
					std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).serverActorInterface->StoreGameStateHash(simulationInputs->timecode, gameStateHash);
				}
			}
		}
	} else if (std::holds_alternative<multiplayer::lockstep::ReplayComponents>(_multiPlayerComponents)) {
		if (auto simulationInputs = std::get<multiplayer::lockstep::ReplayComponents>(_multiPlayerComponents).levelReplayer.GetNextSimulationInputs()) {
			LOG_NETWORK("Simulating inputs from level replayer");
			_proxy.HandleLockstepPlayerInputs(simulationInputs->allInputs);
		}
	} else if constexpr (not GAME_IS_DETERMINISTIC) {
		// ReSharper disable once CppDFAUnreachableCode
		// Integrate physics
		for (auto* world : _level->world) {
			if (world) {
				world->Step(ToDurationF(delta), velocity_iterations, position_iterations);
			}
		}
		// Update positions
		for (auto& phy : _level->physics) {
			for (const auto& body : phy.body) {
				if (body && body->IsEnabled()) {
					auto& obj = phy.GetOwner();
					phy.position = VecFE{body->GetPosition()};
					phy.orientation = FE{body->GetAngle()};
					// Update other components
					if (auto* gfx = obj.TryGetGraphic()) {
						const auto oldGfxPosition = gfx->position;
						gfx->position = body->GetPosition();
						gfx->orientation = phy.orientation.ToFloat();
						// Update draw list if necessary
						if (oldGfxPosition != body->GetPosition()) {
							const auto gfxId = obj.GetGraphicId();
							const auto poolAndDrawList = _level->GetGraphicPoolAndDrawList(gfxId);
							poolAndDrawList.second->QueueUpdate(phy.GetOwnerId(), body->GetPosition());
						}
					}
					if (auto* lig = obj.TryGetLight()) {
						lig->position = body->GetPosition();
					}
					if (auto* snd = obj.TryGetSoundEmitter()) {
						snd->position = body->GetPosition();
					}
					break;
				}
			}
		}
	}
	// Re-sort draw lists
	for (auto& drawList : _level->uprightDrawLists) {
		drawList.Update();
	}
	// If the world is NOT static, the pathfinder's cache should be cleared.
	if constexpr (not ::m2g::Proxy::worldIsStatic) {
		// ReSharper disable once CppDFAUnreachableCode
		_level->pathfinder->clear_cache();
	}
}
void Game::ExecutePostStep(const Stopwatch::Duration& delta) {
	if (IsTurnBasedServer()) {
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
	} else if (IsRealTurnBasedClient()) {
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
		_proxy.OnPostStep(delta);
		ExecuteDeferredActions();
	}

	for (auto& phy : _level->physics) {
		IF(phy.postStep)(phy, delta);
	}
}
void Game::UpdateSounds(const Stopwatch::Duration& delta) {
	for (auto& sound_emitter : _level->soundEmitters) {
		IF(sound_emitter.update)(sound_emitter, delta);
	}
}
void Game::ExecutePreDraw(const Stopwatch::Duration& delta) {
	for (auto& gfx : _level->uprightGraphics) {
		if (gfx.enabled) {
			IF(gfx.preDraw)(gfx, delta);
		}
	}
}
void Game::UpdateHudContents(const Stopwatch::Duration& delta) {
	const auto deltaF = ToDurationF(delta);
	// TODO handle returned actions
	IF(_level->_leftHudUiPanel)->UpdateContents(deltaF);
	IF(_level->_rightHudUiPanel)->UpdateContents(deltaF);
	IF(_level->_messageBoxUiPanel)->UpdateContents(deltaF);
	for (auto &panel : _level->_customNonblockingUiPanels) {
		auto action{panel.UpdateContents(deltaF)};
		action.IfQuit([this] { quit = true; });
		if (auto anyReturnContainer = action.ExtractAnyReturnContainer()) {
			// If UI returned, kill the panel. We cannot delete it yet, the iterator is held by the client, but we
			// can replace it with a killed object that can hold the AnyReturnContainer instead.
			panel.KillWithReturnValue(std::move(*anyReturnContainer));
		}
	}
	IF(_level->_mouseHoverUiPanel)->UpdateContents(deltaF);
	if (_level->_semiBlockingUiPanel) {
		_level->_semiBlockingUiPanel->UpdateContents(deltaF)
				.IfQuit([this] {
					quit = true;
				})
				.IfAnyReturn([this](const ReturnBase&) {
					// The return object is discarded. Remove the state.
					_level->_semiBlockingUiPanel.reset();
				});
	}
}
void Game::ClearBackBuffer() const {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}
void Game::Draw() {
	// Check if only one background layer needs to be drawn
	const auto onlyBackgroundLayerToDraw = [&]() -> std::optional<pb::FlatGraphicsLayer> {
		if (std::holds_alternative<leveleditor::State>(_level->stateVariant)) {
			if (const auto& rightHudName = _level->GetRightHud()->Name();
					rightHudName == "PaintBgRightHud" || rightHudName == "SampleBgRightHud" || rightHudName == "SelectBgRightHud") {
				const auto& levelEditorState = std::get<leveleditor::State>(_level->stateVariant);
				return levelEditorState.GetSelectedBackgroundLayer();
			}
		}
		return std::nullopt;
	}();

	for (const auto& layer : gDrawOrder) {
		// Skip if another background layer needs drawing
		if (onlyBackgroundLayerToDraw && std::holds_alternative<pb::FlatGraphicsLayer>(layer)
				&& *onlyBackgroundLayerToDraw != std::get<pb::FlatGraphicsLayer>(layer)) {
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
				if (auto& gfx = _level->uprightGraphics[gfxId]; gfx.enabled && gfx.draw) {
					IF(gfx.onDraw)(gfx);
				}
			}
		}
	}
}
void Game::DrawLights() {
	for (auto& light : _level->lights) {
		IF(light.onDraw)(light);
	}
}
void Game::ExecutePostDraw(const Stopwatch::Duration& delta) {
	for (auto& gfx : _level->uprightGraphics) {
		if (gfx.enabled) {
			IF(gfx.postDraw)(gfx, delta);
		}
	}
}
void Game::DebugDraw() {
#ifdef DEBUG
	for (int i = 0; i < I(PHYSICS_LAYER_COUNT); ++i) {
		if (_level->world[i]) {
			_level->world[i]->DebugDraw();
		}
	}

	if (IsProjectionTypePerspective(_level->GetProjectionType())) {
		SDL_SetRenderDrawColor(M2_GAME.renderer, 255, 255, 255, 127);
		for (int y = 0; y < 20; ++y) {
			for (int x = 0; x < 20; ++x) {
				m3::VecF p = {x, y, 0};
				if (const auto projected_p = ScreenOriginToProjectionAlongCameraPlaneDstpx(p); projected_p) {
					SDL_RenderDrawPointF(M2_GAME.renderer, projected_p->GetX(), projected_p->GetY());
				}
			}
		}
	}
#endif

	const auto lineHeightM = (ToFloat(systemFontSize) / _dimensions->OutputPixelsPerMeter()) * 1.2f;
	for (const auto& [type, options] : _level->_objectTypeDebugObjects) {
		for (const auto& obj : _level->objects) {
			if (obj.GetType() == type) {
				int monitorLine = 0;
				options.ForEachMonitorValue(obj, [&](const std::string& line) {
					const auto objPosition = obj.InferPositionF();
					SlowDrawSystemTextIn2dWorld(line.c_str(), objPosition - VecF{0.0f, options.monitorOffsetM + ToFloat(monitorLine) * lineHeightM});
					++monitorLine;
				});
			}
		}
	}
	for (const auto& [id, options] : _level->_objectDebugObjects) {
		const auto& obj = _level->objects[id];
		// An object might have monitors due to their type. If so, we should find out how many so that we can skip
		// printing over them.
		const auto initialMonitorLineOffset = [&]() {
			const auto* objTypeDebugOptions = _level->GetObjectTypeDebugOptions(obj.GetType());
			return objTypeDebugOptions ? objTypeDebugOptions->GetMonitorCount() : 0;
		}();
		int monitorLine = initialMonitorLineOffset;
		options.ForEachMonitorValue(obj, [&](const std::string& line) {
			const auto objPosition = obj.InferPositionF();
			SlowDrawSystemTextIn2dWorld(line.c_str(), objPosition - VecF{0.0f, options.monitorOffsetM + ToFloat(monitorLine) * lineHeightM});
			++monitorLine;
		});
	}
}
void Game::DrawHud() {
	IF(_level->_leftHudUiPanel)->Draw();
	IF(_level->_rightHudUiPanel)->Draw();
	IF(_level->_messageBoxUiPanel)->Draw();
	for (auto &panel : _level->_customNonblockingUiPanels) {
		panel.Draw();
	}
	IF(_level->_mouseHoverUiPanel)->Draw();
	IF(_level->_semiBlockingUiPanel)->Draw();
}
void Game::DrawEnvelopes() const {
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
void Game::FlipBuffers() const { SDL_RenderPresent(renderer); }

void Game::OnWindowResize() {
	_dimensions->OnWindowResize();
	if (_level) {
		IF(_level->_leftHudUiPanel)->UpdatePosition();
		IF(_level->_rightHudUiPanel)->UpdatePosition();
		IF(_level->_messageBoxUiPanel)->UpdatePosition();
		for (auto &panel : _level->_customNonblockingUiPanels) {
			panel.UpdatePosition();
		}
		IF(_level->_mouseHoverUiPanel)->UpdatePosition();
		IF (_level->_semiBlockingUiPanel)->UpdatePosition();

		// Clear text label rectangles so that they are regenerated with new size
		for (auto& gfx : _level->uprightGraphics) {
			gfx.textLabelRect = {};
		}
		for (auto& terrainGraphics : std::ranges::reverse_view(_level->flatGraphics)) {
			for (auto& gfx : terrainGraphics) {
				gfx.textLabelRect = {};
			}
		}
	}
}
void Game::SetScale(const float scale) {
	_dimensions->SetScale(scale);
	if (_level) {
		// Clear text label rectangles so that they are regenerated with new size
		for (auto& gfx : _level->uprightGraphics) {
			gfx.textLabelRect = {};
		}
		for (auto& terrainGraphics : std::ranges::reverse_view(_level->flatGraphics)) {
			for (auto& gfx : terrainGraphics) {
				gfx.textLabelRect = {};
			}
		}
	}
}
void Game::SetGameHeightM(const float heightM) {
	_dimensions->SetGameHeightM(heightM);
	if (_level) {
		// Clear text label rectangles so that they are regenerated with new size
		for (auto& gfx : _level->uprightGraphics) {
			gfx.textLabelRect = {};
		}
		for (auto& terrainGraphics : std::ranges::reverse_view(_level->flatGraphics)) {
			for (auto& gfx : terrainGraphics) {
				gfx.textLabelRect = {};
			}
		}
	}
}

void Game::ForEachSprite(const std::function<bool(m2g::pb::SpriteType, const Sprite&)>& op) const {
	for (int i = 0; i < pb::enum_value_count<m2g::pb::SpriteType>(); ++i) {
		const auto type = pb::enum_value<m2g::pb::SpriteType>(i);
		if (const auto& spriteOrTextLabel = GetSpriteOrTextLabel(type);
				std::holds_alternative<Sprite>(spriteOrTextLabel) && not op(type, std::get<Sprite>(spriteOrTextLabel))) {
			return;
		}
	}
}

void Game::ForEachCard(const std::function<bool(m2g::pb::CardType, const Card&)>& op) const {
	for (int i = 0; i < pb::enum_value_count<m2g::pb::CardType>(); ++i) {
		if (const auto type = pb::enum_value<m2g::pb::CardType>(i); not op(type, GetCard(type))) {
			return;
		}
	}
}
std::optional<m2g::pb::SpriteType> Game::GetMainSpriteOfObject(const m2g::pb::ObjectType ot) const {
	const auto objectTypeIndex = pb::enum_index(ot);
	const auto defaultSpriteType = _objectBlueprints[objectTypeIndex].defaultSpriteType;
	if (not defaultSpriteType) {
		return std::nullopt;
	}
	return defaultSpriteType;
}
void Game::ForEachObjectWithMainSprite(const std::function<bool(m2g::pb::ObjectType, m2g::pb::SpriteType)>& op) const {
	for (const auto& objectBlueprint : _objectBlueprints) {
		if (objectBlueprint.defaultSpriteType) {
			if (not op(objectBlueprint.objectType, objectBlueprint.defaultSpriteType)) {
				return;
			}
		}
	}
}
sdl::TextureUniquePtr Game::DrawGameToTexture(const VecF& camera_position) {
	// Temporarily change camera position
	const auto prev_camera_position = GetLevel().GetCamera()->GetPhysique().position;
	GetLevel().GetCamera()->GetPhysique().position = VecFE{camera_position};

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
	GetLevel().GetCamera()->GetPhysique().position = prev_camera_position;

	return render_target;
}
bool Game::IsMouseOnAnyUiPanel() const {
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

void Game::RecalculateDirectionalAudio() {
	if (_level->leftListener || _level->rightListener) {
		// Loop over sounds
		for (auto& sound_emitter : _level->soundEmitters) {
			const auto& sound_position = sound_emitter.position;
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

void Game::QueueCommand(std::string cmd) {
	_queuedCommands.emplace_back(std::move(cmd));
}
void Game::AddDeferredAction(const std::function<void()>& action) {
	_level->deferredActions.push(action);
}
void Game::ExecuteDeferredActions() {
	// Execute deferred actions one by one. A deferred action may insert another deferred action into the queue.
	// Thus, we cannot iterate over the queue, we must pop one by one.
	while (not _level->deferredActions.empty()) {
		_level->deferredActions.front()();
		_level->deferredActions.pop();
	}
}

Game::CommandResult Game::ExecuteCommand(const std::string& cmd) {
	// Make sure this function is called while the events are being handled
	if (not _eventsAreBeingHandled) {
		throw M2_ERROR("Attempt to execute command outside of handling events");
	}

	std::optional<std::string_view> command;
	std::array<std::string_view, 16> argument;
	int argCount = 0;

	using std::operator""sv;
	for (const auto wordSubrange : std::views::split(GetTrimmedView(cmd), " "sv)) {
		if (const auto word = GetTrimmedView(std::string_view{wordSubrange}); not word.empty()) {
			if (not command) {
				command = word;
			} else {
				argument[argCount++] = word;
			}
		}
	}
	if (not command || command->empty()) {
		return CommandSuccess{};
	}

	std::string arguments;
	for (int i = 0; i < argCount; ++i) {
		arguments += argument[i];
		arguments += ' ';
	}
	LOG_INFO("Executing command with arguments", *command, arguments);

	if (*command == "game") {
		if (const auto result = _proxy.ExecuteGameCommand(argument)) {
			return CommandSuccess{};
		} else {
			return CommandFail{.error = result.error()};
		}
	} else if (*command == "LoadLevelEditor") {
		if (not argument[0].empty()) {
			if (const auto result = M2_GAME.LoadLevelEditor((M2_GAME.GetResources().GetLevelsDir() / argument[0]).string())) {
				return CommandSuccess{.levelReplaced = true};
			} else {
				return CommandFail{.error = result.error()};
			}
		} else {
			return CommandFail{.error = "Missing argument"};
		}
	} else if (*command == "LoadSheetEditor") {
		if (const auto result = M2_GAME.LoadBulkSheetEditor()) {
			// Execute main menu the first time the bulk sheet editor is run
			UiPanel::create_and_run_blocking(&bulksheeteditor::gMainMenu);
			return CommandSuccess{.levelReplaced = true};
		} else {
			return CommandFail{.error = result.error()};
		}
	} else if (*command == "LoadSpriteEditor") {
		if (const auto result = M2_GAME.LoadSheetEditor()) {
			// Execute main menu the first time the sheet editor is run
			UiPanel::create_and_run_blocking(&sheet_editor_main_menu);
			return CommandSuccess{.levelReplaced = true};
		} else {
			return CommandFail{.error = result.error()};
		}
	} else if (*command == "MoveBackground") {
		if (not argument[0].empty() && not argument[1].empty() && not argument[2].empty()) {
			const auto layerFrom = strtol(std::string{argument[0]}.c_str(), nullptr, 0);
			const auto layerTo = strtol(std::string{argument[1]}.c_str(), nullptr, 0);
			const auto level = std::string{argument[2]};
			if (const auto result = MoveBackground(I(layerFrom), I(layerTo), level)) {
				return CommandSuccess{};
			} else {
				return CommandFail{.error = result.error()};
			}
		} else {
			return CommandFail{.error = "Missing argument"};
		}
	} else if (*command == "ReplayLockstep") {
		if (not argument[0].empty()) {
			if (const auto result = M2_GAME.ReplayLockstep(std::string{argument[0]})) {
				return CommandSuccess{.levelReplaced = true};
			} else {
				return CommandFail{.error = result.error()};
			}
		} else {
			return CommandFail{.error = "Missing argument"};
		}
	}

	return UnknownCommand{};
}
