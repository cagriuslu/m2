#pragma once
#include "audio/AudioManager.h"
#include <m2/containers/DrawList.h>
#include "Events.h"
#include "Card.h"
#include "Level.h"
#include "Meta.h"
#include "Proxy.h"
#include "audio/Song.h"
#include <m2/video/Sprite.h>
#include "game/Animation.h"
#include "multi_player/TurnBasedHostClientThread.h"
#include <m2/video/TextLabel.h>
#include "multi_player/TurnBasedRealClientThread.h"
#include "multi_player/TurnBasedBotClientThread.h"
#include "multi_player/TurnBasedServerActorInterface.h"
#include "multi_player/TurnBasedServerComponents.h"
#include "protobuf/MessageLUT.h"
#include <m2g_ObjectType.pb.h>
#include <m2g/Proxy.h>
#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <filesystem>
#include <functional>
#include <vector>
#include "math/Hash.h"
#include "GameDimensions.h"
#include <m2g_KeyType.pb.h>
#include <m2/video/Shape.h>
#include <m2/GameResources.h>
#include <m2/multi_player/lockstep/ServerComponents.h>

#define M2_GAME (m2::Game::Instance())
#define M2_DEFER(f) (M2_GAME.AddDeferredAction(f))
#define M2G_PROXY (M2_GAME.Proxy())
#define M2_LEVEL (M2_GAME.GetLevel())
#define M2_PLAYER (*M2_LEVEL.GetPlayer())

namespace m2 {
	/// \brief Container of all objects managed by the game engine.
	/// \details This class is designed to be a singleton, and an instance is created by the main function. `M2_GAME`
	/// macro can be used to easily access the instance.
	class Game {
		static Game* _instance;
		::m2g::Proxy _proxy{};
		GameResources _resources{_proxy.gameIdentifier, _proxy.defaultFontPath};
		std::optional<GameDimensions> _dimensions;

		mutable std::optional<VecF> _mouse_position_world_m;
		mutable std::optional<VecF> _screen_center_to_mouse_position_m;  // Doesn't mean much in 2.5D mode

		std::variant<std::monostate, TurnBasedServerComponents, network::TurnBasedRealClientThread, multiplayer::lockstep::ServerComponents, multiplayer::lockstep::ClientActorInterface> _multiPlayerComponents;
		bool _server_update_necessary{}, _server_update_with_shutdown{};
		std::optional<network::SequenceNo> _lastSentOrReceivedServerUpdateSequenceNo;

		////////////////////////////////////////////////////////////////////////
		////////////////////////////// RESOURCES ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		Rational _font_letter_width_to_height_ratio; // letter w/h // TODO get rid of this, because this is only accurate if the font is used in default size, which isn't always the case
		std::optional<TextLabelCache> _textLabelCache;
		std::optional<ShapeCache> _shapeCache;
		std::vector<std::variant<Sprite, pb::TextLabel>> _sprites;
		std::vector<ObjectBlueprint> _objectBlueprints;

	public:  // TODO private
		std::optional<Level> _level;

		static void CreateInstance();
		static bool HasInstance() { return _instance; }
		static Game& Instance() { return *_instance; }
		static void DestroyInstance();

		::m2g::Proxy& Proxy() { return _proxy; }

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// WINDOW ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		SDL_Window* window{};
		SDL_Cursor* cursor{};
		SDL_Renderer* renderer{};
		SDL_Texture* light_texture{};
		std::optional<AudioManager> audio_manager;
		uint32_t pixel_format{};
		TTF_Font* font{};
		bool quit{};

		////////////////////////////////////////////////////////////////////////
		////////////////////////////// RESOURCES ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		std::optional<pb::SpriteSheets> spriteSheetsPb;
		std::vector<SpriteSheet> spriteSheets;
		std::optional<SpriteEffectsSheet> spriteEffectsSheet;
		std::vector<m2g::pb::SpriteType> level_editor_background_sprites;
		pb::MessageLUT<pb::Card, Card> cards;
		pb::MessageLUT<pb::Animation, Animation> animations;
		pb::MessageLUT<pb::Song, Song> songs;
		std::multimap<m2g::pb::KeyType, SDL_Scancode> keyToScancodeMap;
		std::map<SDL_Scancode, m2g::pb::KeyType> scancodeToKeyMap;
		const Rational& FontLetterWidthToHeightRatio() const { return _font_letter_width_to_height_ratio; }
		TextLabelCache& GetTextLabelCache() { return *_textLabelCache; }
		ShapeCache& GetShapeCache() { return *_shapeCache; }

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// CONFIG ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		static constexpr int velocity_iterations{8};
		static constexpr int position_iterations{3};
		static constexpr float max_hearing_distance_m{20.0f};
		static constexpr float min_hearing_facing_away{0.1f};  // Ratio

		////////////////////////////////////////////////////////////////////////
		///////////////////////////////// MISC /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		Events events;

		std::vector<std::string> console_output;

		Game();
		~Game();

		// Pre-game management

		bool IsMultiPlayer() const { return not std::holds_alternative<std::monostate>(_multiPlayerComponents); }
		bool IsTurnBasedMultiPlayer() const { return std::holds_alternative<TurnBasedServerComponents>(_multiPlayerComponents) || std::holds_alternative<network::TurnBasedRealClientThread>(_multiPlayerComponents); }
		void_expected HostTurnBasedGame(unsigned max_connection_count);
		void_expected HostLockstepGame(unsigned max_connection_count);
		/// For client
		void_expected JoinTurnBasedGame(const std::string& addr);
		void_expected JoinLockstepGame(const std::string& addr);
		/// For client
		void LeaveGame();
		/// For server
		bool AddBot();
		/// For server
		network::TurnBasedBotClientThread& FindBot(int receiver_index);
		bool IsTurnBasedServer() const { return std::holds_alternative<TurnBasedServerComponents>(_multiPlayerComponents); }
		bool IsRealTurnBasedClient() const { return std::holds_alternative<network::TurnBasedRealClientThread>(_multiPlayerComponents); }
		TurnBasedServerActorInterface& ServerThread() { return *std::get<TurnBasedServerComponents>(_multiPlayerComponents).serverActorInterface; }
		network::TurnBasedHostClientThread& TurnBasedHostClientThread() { return *std::get<TurnBasedServerComponents>(_multiPlayerComponents).hostClientThread; }
		network::TurnBasedRealClientThread& TurnBasedRealClientThread() { return std::get<network::TurnBasedRealClientThread>(_multiPlayerComponents); }
		multiplayer::lockstep::ServerActorInterface& GetLockstepServerActor() { return *std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).serverActorInterface; }
		multiplayer::lockstep::ClientActorInterface& GetLockstepHostClientActor() { return *std::get<multiplayer::lockstep::ServerComponents>(_multiPlayerComponents).hostClientActorInterface; }
		multiplayer::lockstep::ClientActorInterface& GetLockstepGuestClientActor() { return std::get<multiplayer::lockstep::ClientActorInterface>(_multiPlayerComponents); }
		multiplayer::lockstep::ClientActorInterface& GetLockstepClientActor();
		std::optional<network::SequenceNo> LastServerUpdateSequenceNo() const { return _lastSentOrReceivedServerUpdateSequenceNo; }
		int GetTotalPlayerCount();
		int GetSelfIndex();
		int GetTurnBasedTurnHolderIndex();
		bool IsOurTurn();
		void QueueClientCommand(const m2g::pb::TurnBasedClientCommand& cmd);

		// Level management

		void_expected LoadSinglePlayer(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& level_name = "");
		void_expected LoadTurnBasedMultiPlayerAsHost(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& level_name = "");
		void_expected LoadTurnBasedMultiPlayerAsGuest(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& level_name = "");
		/// Load lockstep level using the game init parameters received from the server (or decided locally if this instance is the server).
		void_expected LoadLockstep(const std::variant<std::filesystem::path, pb::Level>& levelPathOrBlueprint, const std::string& levelName, const m2g::pb::LockstepGameInitParams& gameInitParams);
		void_expected LoadLevelEditor(const std::string& level_resource_path);
		void_expected LoadSheetEditor();
		void_expected LoadBulkSheetEditor();
		bool HasLevel() const { return static_cast<bool>(_level); }
		Level& GetLevel() { return *_level; }
		void UnloadLevel() { _level.reset(); ResetState(); }

		// Accessors

		/// Platform independent hash of the game identifier for different peers to recognize each other.
		int32_t Hash() const { return HashI(_proxy.gameIdentifier); }
		const GameResources& GetResources() const { return _resources; }
		const GameDimensions& Dimensions() const { return *_dimensions; }
		const std::variant<Sprite,pb::TextLabel>& GetSpriteOrTextLabel(const m2g::pb::SpriteType sprite_type) const { return _sprites[pb::enum_index(sprite_type)]; }
		void ForEachSprite(const std::function<bool(m2g::pb::SpriteType, const Sprite&)>& op) const;
		const Card& GetCard(const m2g::pb::CardType card_type) const { return cards[card_type]; }
		void ForEachCard(const std::function<bool(m2g::pb::CardType, const Card&)>& op) const;
		std::optional<m2g::pb::SpriteType> GetMainSpriteOfObject(m2g::pb::ObjectType) const;
		void ForEachObjectWithMainSprite(const std::function<bool(m2g::pb::ObjectType, m2g::pb::SpriteType)>& op) const;
		[[nodiscard]] VecI MousePositionPx() const { return events.MousePosition(); }
		const VecF& MousePositionWorldM() const; // TODO move into Level?
		const VecF& ScreenCenterToMousePositionM() const; // TODO move into Level?
		sdl::TextureUniquePtr DrawGameToTexture(const VecF& camera_position); // TODO move into Level?
		/// This function can be used to check if the mouse is resting on any UI panels. If not, the mouse must be
		/// inside the game window.
		[[nodiscard]] bool IsMouseOnAnyUiPanel() const;

		// Handlers

		void HandleQuitEvent();
		void HandleWindowResizeEvent();
		void HandleConsoleEvent();
		void HandlePauseEvent();
		void HandleHudEvents();
		void HandleNetworkEvents();
		bool ShouldSimulatePhysics();
		void ExecutePreStep(const Stopwatch::Duration& delta);
		void UpdateCharacters(const Stopwatch::Duration& delta);
		void ExecuteStep(const Stopwatch::Duration& delta);
		void ExecutePostStep(const Stopwatch::Duration& delta);
		void CalculateGameStateHash();
		void UpdateSounds(const Stopwatch::Duration& delta);
		void ExecutePreDraw(const Stopwatch::Duration& delta);
		void UpdateHudContents(const Stopwatch::Duration& delta);
		void ClearBackBuffer() const;
		void Draw();
		void DrawLights();
		void ExecutePostDraw(const Stopwatch::Duration& delta);
		void DebugDraw();
		void DrawHud();
		void DrawEnvelopes() const;
		void FlipBuffers() const;

		// Modifiers

		void OnWindowResize();
		/// Scale can be used to adjust the zoom
		void SetScale(float scale);
		/// Game height in meters can be used to adjust the zoom
		void SetGameHeightM(float heightM);
		void ResetMousePosition();
		void RecalculateDirectionalAudio();

		void AddDeferredAction(const std::function<void()>& action);
		void ExecuteDeferredActions();

	   private:
		void ResetState();
		void RecalculateMousePosition() const;
	};
}  // namespace m2
