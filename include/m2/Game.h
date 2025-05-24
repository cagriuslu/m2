#pragma once
#include "audio/AudioManager.h"
#include <m2/containers/DrawList.h>
#include "Events.h"
#include "Item.h"
#include "Level.h"
#include "Meta.h"
#include "Object.h"
#include "Proxy.h"
#include "audio/Song.h"
#include <m2/video/Sprite.h>
#include "game/Animation.h"
#include "network/HostClientThread.h"
#include <m2/video/TextLabel.h>
#include "network/RealClientThread.h"
#include "network/BotClientThread.h"
#include "network/ServerThread.h"
#include "protobuf/LUT.h"
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

#define M2_GAME (m2::Game::Instance())
#define M2_DEFER(f) (M2_GAME.AddDeferredAction(f))
#define M2G_PROXY (M2_GAME.Proxy())
#define M2_LEVEL (M2_GAME.GetLevel())
#define M2_PLAYER (*M2_LEVEL.Player())

namespace m2 {
	// Client server comes after server thread, thus during shutdown, it'll be killed before the ServerThread.
	// This is important for the server thread to not linger too much.
	using ServerThreads = std::pair<network::ServerThread, network::HostClientThread>;
	using BotAndIndexThread = std::pair<network::BotClientThread,int>;

	class Game {
		static Game* _instance;
		::m2g::Proxy _proxy{};
		std::optional<GameDimensionsManager> _dimensionsManager;

		mutable std::optional<VecF> _mouse_position_world_m;
		mutable std::optional<VecF> _screen_center_to_mouse_position_m;  // Doesn't mean much in 2.5D mode

		std::variant<std::monostate, ServerThreads, network::RealClientThread> _multi_player_threads;
		std::list<BotAndIndexThread> _bot_threads; // thread,receiver_index pairs (receiver_index is initially -1)
		bool _server_update_necessary{}, _server_update_with_shutdown{};
		std::optional<SequenceNo> _lastSentOrReceivedServerUpdateSequenceNo;

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
		float _delta_time_s{};

		static void CreateInstance();
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
		//////////////////////////////// PATHS /////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		std::filesystem::path resource_dir;
		std::filesystem::path levels_dir;
		std::filesystem::path spriteSheetsPath;

		////////////////////////////////////////////////////////////////////////
		////////////////////////////// RESOURCES ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		std::vector<SpriteSheet> spriteSheets;
		std::optional<SpriteEffectsSheet> spriteEffectsSheet;
		std::vector<m2g::pb::SpriteType> level_editor_background_sprites;
		pb::LUT<pb::Item, NamedItem> named_items;
		pb::LUT<pb::Animation, Animation> animations;
		pb::LUT<pb::Song, Song> songs;
		std::multimap<m2g::pb::KeyType, SDL_Scancode> keyToScancodeMap;
		std::map<SDL_Scancode, m2g::pb::KeyType> scancodeToKeyMap;
		const Rational& FontLetterWidthToHeightRatio() const { return _font_letter_width_to_height_ratio; }
		TextLabelCache& GetTextLabelCache() { return *_textLabelCache; }
		ShapeCache& GetShapeCache() { return *_shapeCache; }

		////////////////////////////////////////////////////////////////////////
		//////////////////////////////// CONFIG ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		static constexpr unsigned phy_frequency{100};
		static constexpr float phy_period{1.0f / static_cast<float>(phy_frequency)};
		static constexpr unsigned phy_period_ticks{1000u / phy_frequency};
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

		int32_t Hash() const { return ihash(_proxy.game_identifier); }
		bool IsMultiPlayer() const { return not std::holds_alternative<std::monostate>(_multi_player_threads); }
		/// For server
		void_expected HostGame(mplayer::Type type, unsigned max_connection_count);
		/// For client
		void_expected JoinGame(mplayer::Type type, const std::string& addr);
		/// For client
		void LeaveGame();
		/// For server
		bool AddBot();
		/// For server
		network::BotClientThread& FindBot(int receiver_index);
		bool IsServer() const { return std::holds_alternative<ServerThreads>(_multi_player_threads); }
		bool IsRealClient() const { return std::holds_alternative<network::RealClientThread>(_multi_player_threads); }
		network::ServerThread& ServerThread() { return std::get<ServerThreads>(_multi_player_threads).first; }
		network::HostClientThread& HostClientThread() { return std::get<ServerThreads>(_multi_player_threads).second; }
		network::RealClientThread& RealClientThread() { return std::get<network::RealClientThread>(_multi_player_threads); }
		std::optional<SequenceNo> LastServerUpdateSequenceNo() const { return _lastSentOrReceivedServerUpdateSequenceNo; }
		int TotalPlayerCount();
		int SelfIndex();
		int TurnHolderIndex();
		bool IsOurTurn();
		void QueueClientCommand(const m2g::pb::ClientCommand& cmd);
		// Level management
		void_expected LoadSinglePlayer(const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name = "");
		void_expected LoadMultiPlayerAsHost(const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name = "");
		void_expected LoadMultiPlayerAsGuest(const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& level_name = "");
		void_expected LoadLevelEditor(const std::string& level_resource_path);
		void_expected LoadSheetEditor();
		void_expected LoadBulkSheetEditor();
		bool HasLevel() const { return static_cast<bool>(_level); }
		Level& GetLevel() { return *_level; }
		void UnloadLevel() { _level.reset(); ResetState(); }

		// Accessors

		const GameDimensionsManager& Dimensions() const { return *_dimensionsManager; }
		const std::variant<Sprite,pb::TextLabel>& GetSpriteOrTextLabel(const m2g::pb::SpriteType sprite_type) const { return _sprites[pb::enum_index(sprite_type)]; }
		void ForEachSprite(const std::function<bool(m2g::pb::SpriteType, const Sprite&)>& op) const;
		const NamedItem& GetNamedItem(const m2g::pb::ItemType item_type) const { return named_items[item_type]; }
		void ForEachNamedItem(const std::function<bool(m2g::pb::ItemType, const NamedItem&)>& op) const;
		std::optional<m2g::pb::SpriteType> GetMainSpriteOfObject(m2g::pb::ObjectType) const;
		void ForEachObjectWithMainSprite(const std::function<bool(m2g::pb::ObjectType, m2g::pb::SpriteType)>& op) const;
		float DeltaTimeS() const { return _delta_time_s; }
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
		void ExecutePreStep();
		void UpdateCharacters();
		void ExecuteStep();
		void ExecutePostStep();
		void UpdateSounds();
		void ExecutePreDraw();
		void UpdateHudContents();
		void ClearBackBuffer() const;
		void DrawBackground();
		void DrawForeground();
		void DrawLights();
		void ExecutePostDraw();
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
		void ResetMousePosition() { _mouse_position_world_m = std::nullopt; _screen_center_to_mouse_position_m = std::nullopt; }
		void RecalculateDirectionalAudio();

		void AddDeferredAction(const std::function<void()>& action);
		void ExecuteDeferredActions();

	   private:
		void ResetState();
		void RecalculateMousePosition() const;
	};
}  // namespace m2
