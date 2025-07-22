#pragma once
#include "Controls.h"
#include "Group.h"
#include "Object.h"
#include "ui/UiPanelBlueprint.h"
#include "math/Rational.h"
#include "component/Graphic.h"
#include "Meta.h"
#include <m2g_GroupType.pb.h>
#include <Network.pb.h>
#include <Level.pb.h>
#include <string_view>
#include <array>
#include "network/SequenceNo.h"
#include "ui/MessageBox.h"
#include "ObjectBlueprint.h"

namespace m2 {
	class Proxy {
	   public:
		// TODO make these private, give access to Game, implement getters
		Proxy() = default;
		virtual ~Proxy() = default;

		/// Short identifier of the game which is used to discover the resources.
		const std::string gameIdentifier = "<game-identifier>";
		/// Friendly name of the game that's use as the window title.
		const std::string gameFriendlyName = "<game-friendly-name>";

		/// PPM (pixels per meter) of the overall game. Ideally, this number should be the greatest common factor of all
		/// sprites used in the game. In case there could be outliers, this values isn't calculated from Sprite Sheets,
		/// but instead hardcoded in Proxy.
		const int gamePpm = 16;
		const int gameAspectRatioMul = 5;
		const int gameAspectRatioDiv = 4;
		/// Determines if nearest neighbor should be used while resizing sprites
		const bool areGraphicsPixelated = true;
		/// Determines the initial and minimum window size.
		const float defaultGameHeightM = 18.0f;

		// const std::string defaultFontPath = "fonts/Courier_Prime/CourierPrime-Regular.ttf";
		// const std::string defaultFontPath = "fonts/Kode_Mono/KodeMono-VariableFont_wght.ttf";
		// const std::string defaultFontPath = "fonts/Martian_Mono/MartianMono-VariableFont_wdth,wght.ttf";
		// const std::string defaultFontPath = "fonts/Roboto_Mono/RobotoMono-VariableFont_wght.ttf";
		// const std::string defaultFontPath = "fonts/Share_Tech_Mono/ShareTechMono-Regular.ttf";
		// const std::string defaultFontPath = "fonts/Syne_Mono/SyneMono-Regular.ttf";
		const std::string defaultFontPath = "fonts/VT323/VT323-Regular.ttf";
		// const std::string defaultFontPath = "fonts/Xanh_Mono/XanhMono-Regular.ttf";
		const int default_font_size = 240;

		/// Should the world have gravity towards downwards direction
		const VecF gravity{};

		/// Is the world (background, the obstacles) static? If true, pathfinder uses caching.
		const bool world_is_static = true;

		/// Is lightning enabled? Darkens the textures.
		const bool lightning = false;

		/// Z component of the focus position, which has the same XY position as the player
		float focus_point_height = 2.0f;

		/// Z component of the xy-plane for camera to mouse ray-casts
		const float xy_plane_z_component = 0.75f;

		/// Is the camera also a sound listener
		const bool camera_is_listener = false;

		/// Dimming factor of the graphics when dimming is enabled
		const float dimming_factor = 0.333f;

		/// Describes the details of the objects
		const std::vector<ObjectBlueprint> objectBlueprints;

		void load_resources() {}

		// UI
		const UiPanelBlueprint* MainMenuBlueprint() { return nullptr; }
		const UiPanelBlueprint* PauseMenuBlueprint() { return nullptr; }
		const UiPanelBlueprint* LeftHudBlueprint() { return nullptr; }
		const UiPanelBlueprint* RightHudBlueprint() { return nullptr; }
		/// The MessageBox blueprint should contain a Widget named "MessageText" of variant TextBlueprint. This widget
		/// will be filled with the message text.
		std::pair<const UiPanelBlueprint*, std::variant<std::monostate,RectI,RectF>> MessageBoxBlueprintAndArea() { return std::make_pair(&DefaultMessageBoxBlueprint, DefaultMessageBoxArea); }

		/// Called before/after a single player level is loaded
		void pre_single_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}
		void post_single_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}

		/// These functions must be identical between the host and the client, because the levels are expected to be
		/// identical after they are initialized.
		void pre_multi_player_level_client_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}
		void post_multi_player_level_client_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}

		/// After the levels are initialized (identically), host can populate the random parts of the level, after which
		/// second TurnBasedServerUpdate will be published.
		void multi_player_level_server_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {}

		/// Maps 0-based client indexes to the IDs of the objects that represent a client in this game instance.
		/// While loading the level, this vector should be filled with IDs of identical player objects.
		/// Then in post_multi_player_level_client_init, M2_GAME.client_thread().receiver_index() can be queried to
		/// learn the 0-based index of this game instance, and the corresponding object can be assigned to M2_PLAYER.
		/// For the server, the first item would contain the ObjectId of the player.
		/// For the client with index 1, the second item would contain the ObjectId of the player.
		std::vector<m2::ObjectId> multiPlayerObjectIds;

		/// Should be implemented from the perspective of a server. Implementation should return the new turn holder
		/// index if the command is accepted and a TurnBasedServerUpdate is necessary. Implementation should return std::nullopt
		/// if the command should be ignored and no TurnBasedServerUpdate is necessary. Implementation should return -1 if the
		/// game ended (one final TurnBasedServerUpdate is sent in this case).
		std::optional<int> handle_client_command(MAYBE int turn_holder_index, MAYBE const m2g::pb::TurnBasedClientCommand& client_command) { return std::nullopt; }
		/// Should be implemented from the perspective of a client.
		void handle_server_command(MAYBE const m2g::pb::TurnBasedServerCommand& server_command) {}
		/// Should be implemented from the perspective of a client. For the server, this function is called after the
		/// TurnBasedServerUpdate is published (except the initial TurnBasedServerUpdate). For the client, it's called after the
		/// TurnBasedServerUpdate is received & processed by the engine. If shutdown is true, the shutdown flag is set in the
		/// TurnBasedServerUpdate and the server or the client will be shutdown after this call.
		void post_server_update(MAYBE SequenceNo sequenceNo, MAYBE bool shutdown) {}
		/// Should be implemented from the perspective of a bot.
		void bot_handle_server_update(MAYBE const m2::pb::TurnBasedServerUpdate& server_update) {}
		/// Should be implemented from the perspective of a bot.
		void bot_handle_server_command(MAYBE const m2g::pb::TurnBasedServerCommand& server_command, MAYBE int receiver_index) {}
		/// Should be implemented from the perspective of a real client. The client has disconnected from the server and
		/// 30 seconds has passed while trying to reconnect. Implementation should display the appropriate message and
		/// return. Once returned, current level will be destroyed and main menu will be triggerred.
		void handle_disconnection_from_server() {}
		/// Should be implemented from the perspective of a real client. The server has behaved unexpectedly, and it's
		/// possibly not a recognized server. Implementation should display the appropriate message and return. Once
		/// returned, current level will be destroyed and main menu will be triggerred.
		void handle_unrecognized_server() {}
		/// Should be implemented from the perspective of a server. The client with the given index has disconnected
		/// from the server and hasn't reconnected for 15 seconds, or hasn't signalled as ready for another 15 seconds.
		/// Implementation should return true if the client should be replaced with a bot. Implementation should return
		/// false if the level should be destroyed and main menu is triggered.
		bool handle_disconnected_client(MAYBE int receiver_index) { return false; }
		/// Should be implemented from the perspective of a server. The client with the given index has misbehaved
		/// (sent invalid/unexpected message). Implementation should return true if the client should be replaced with a
		/// bot. Implementation should return false if level should be destroyed and main menu triggerred.
		bool handle_misbehaving_client(MAYBE int receiver_index) { return false; }

		/// This function is called if a background tile has a fixture. The FixtureDefinition, except the shape, should
		/// be filled by the game code.
		third_party::physics::FixtureDefinition TileFixtureDefinition(MAYBE m2g::pb::SpriteType spriteType) { throw M2_ERROR("Proxy is missing tile fixture definition"); }
		/// Called after a tile is created
		void post_tile_create(MAYBE m2::Object& obj, MAYBE m2g::pb::SpriteType sprite_type) {}

		/// Load foreground object from a level blueprint
		m2::void_expected LoadForegroundObjectFromLevelBlueprint(MAYBE m2::Object& obj) { throw M2_ERROR("Proxy is missing an object loader"); }
		/// Load foreground object from a TurnBasedServerUpdate. The item and resources should NOT be added to the character,
		/// because those will be added automatically after the initialization.
		m2::void_expected init_server_update_fg_object(MAYBE m2::Object& obj, MAYBE const std::vector<m2g::pb::ItemType>& named_item_types, MAYBE const std::vector<m2::pb::Resource>& resources) { return {}; }

		/// Create Group for the given type
		m2::Group* create_group(MAYBE m2g::pb::GroupType group_type) { throw M2_ERROR("Proxy is missing a group factory"); }

		/// A callback that's called every pre-step. Since this callback is not tied to any object, object creating and
		/// destruction can be done here safely.
		void OnPreStep() {}
		/// A callback that's called every pre-step.
		void OnPostStep() {}
	};
}
