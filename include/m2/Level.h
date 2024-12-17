#pragma once
#include <Level.pb.h>
#include <box2d/b2_world.h>
#include <m2/game/DynamicGridLinesLoader.h>
#include <m2g/Proxy.h>

#include <functional>
#include <optional>
#include <string>
#include <m2/sdl/Detail.h>
#include <m2/containers/DrawList.h>
#include "Meta.h"
#include "Object.h"
#include "bulk_sheet_editor/State.h"
#include "level_editor/Detail.h"
#include "level_editor/State.h"
#include "m2/game/Pathfinder.h"
#include "m3/VecF.h"
#include "multi_player/State.h"
#include "multi_player/Type.h"
#include "pixel_editor/State.h"
#include "sheet_editor/DynamicImageLoader.h"
#include "sheet_editor/State.h"
#include "single_player/State.h"
#include "ui/Panel.h"
#include <queue>

namespace m2 {
	// Forward iterator
	class Game;

	class Level final {
		std::optional<std::filesystem::path> _lb_path;
		std::optional<pb::Level> _lb;
		std::string _name;

		RectI _background_boundary;
		std::list<ui::Panel> _custom_nonblocking_ui_panels;
		std::optional<std::set<ObjectId>> _dimming_exceptions;
		bool _is_panning{};

	public:
		~Level();

		// Objects are not meant to be iterated over, as it holds all types of objects. If a certain type of object
		// needs to be iterated over, create a component, attach the component to an object, put component in separate
		// pool, and iterate over that pool. Another reason to put a component inside a Pool: if the type of object that
		// is using that component is created/destroyed very rapidly.
		Pool<Object> objects;
		std::map<GroupId, std::unique_ptr<Group>, GroupId::Less> groups;
		DrawList draw_list;
		Pool<Physique> physics;
		Pool<Graphic> graphics;
		std::array<Pool<Graphic>, static_cast<int>(BackgroundLayer::n)> terrain_graphics;  // First pool is the front-most terrain
		Pool<Light> lights;
		Pool<SoundEmitter> sound_emitters;
		Pool<CharacterVariant> characters;
		b2World* world{};
		box2d::ContactListener* contact_listener{};
		Id camera_id{}, player_id{}, pointer_id{};
		std::optional<SoundListener> left_listener, right_listener;
		std::optional<Pathfinder> pathfinder;

		std::optional<ui::Panel> left_hud_ui_panel, right_hud_ui_panel;
		// Rect represents the position ratio of the UI in reference to the game_and_hud dimensions
		// If there's an active blocking panel, all events are delivered to it. World is still simulated, but button and mouse presses won't be delivered to the world objects.
		std::optional<ui::Panel> custom_blocking_ui_panel;
		std::optional<std::string> message;
		std::optional<ui::Panel> message_box_ui_panel;

		std::optional<sdl::ticks_t> level_start_ticks;
		std::optional<sdl::ticks_t> level_start_pause_ticks;
		std::queue<std::function<void()>> deferred_actions;
		std::variant<
		    std::monostate, splayer::State, mplayer::State, ledit::State, pedit::State, sedit::State, bsedit::State>
		    type_state;
		std::optional<DynamicGridLinesLoader> dynamic_grid_lines_loader;
		std::optional<DynamicGridLinesLoader> dynamic_sheet_grid_lines_loader;

		void_expected init_single_player(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name);
		void_expected init_multi_player_as_host(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name);
		void_expected init_multi_player_as_guest(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name);
		void_expected init_level_editor(const std::filesystem::path& lb_path);
		void_expected init_pixel_editor(const std::filesystem::path& path, int x_offset, int y_offset);
		void_expected init_sheet_editor(const std::filesystem::path& path);
		void_expected init_bulk_sheet_editor(const std::filesystem::path& path);
		void_expected reset_sheet_editor();
		void_expected reset_bulk_sheet_editor();

		// Accessors
		std::optional<std::filesystem::path> path() const { return _lb_path; }
		std::optional<pb::Level> level_blueprint() const { return _lb; }
		const std::string& name() const { return _name; }
		/// Inclusive rectangle that contains all terrain graphics inside. The unit is meters.
		[[nodiscard]] const RectI& background_boundary() const { return _background_boundary; }
		const std::string& identifier() const { return _lb ? _lb->identifier() : empty_string; }
		pb::ProjectionType projection_type() const {
			return ((std::holds_alternative<splayer::State>(type_state) ||
			         std::holds_alternative<mplayer::State>(type_state)) &&
			        _lb)
			    ? _lb->projection_type()
			    : pb::ProjectionType::PARALLEL;
		}
		m3::VecF camera_offset() const {
			return _lb
			    ? m3::VecF{projection_type() == pb::PERSPECTIVE_XYZ ? _lb->camera_offset() : 0.0f, _lb->camera_offset(), _lb->camera_z_offset()}
			    : m3::VecF{};
		}
		float horizontal_fov() const;
		Object* player() { return objects.get(player_id); }
		Object* camera() { return objects.get(camera_id); }
		sdl::ticks_t get_level_duration() const;

		// Modifiers

		void begin_game_loop();

		// Features

		/// "Dimming with exceptions" is a mod where sprite sheets are dimmed for all objects except the exceptions.
		const std::optional<std::set<ObjectId>>& dimming_exceptions() const { return _dimming_exceptions; }
		void enable_dimming_with_exceptions(std::set<ObjectId>&& exceptions);
		void disable_dimming_with_exceptions();

		/// Show the HUD UI elements. HUD is set to be shown at start of the game.
		void enable_hud();
		/// Hides the HUD UI elements. UI elements would get disabled, thus they won't receive any events or updates.
		void disable_hud();

		/// Adds a UI element that is drawn above the HUD. The UI doesn't block the game loop and consumes only the
		/// events meant for itself.
		template <typename... Args>
		std::list<ui::Panel>::iterator add_custom_nonblocking_ui_panel(Args&&... args) {
			return _custom_nonblocking_ui_panels.emplace(_custom_nonblocking_ui_panels.end(), std::forward<Args>(args)...);
		}
		/// Removes the custom UI immediately. Can be called from the UI itself if the UI blueprint is static
		/// (won't cause lambdas to be deallocated). Can be called from outside the UI safely.
		void remove_custom_nonblocking_ui_panel(std::list<ui::Panel>::iterator it);
		/// Removes the custom UI at next step. Can be called from anywhere.
		void remove_custom_nonblocking_ui_panel_deferred(std::list<ui::Panel>::iterator it);

		/// Displays a UI element as a blocking panel, above the HUD. The UI doesn't block the game loop but consumes all events
		/// except the time passed event. Mouse movement, button and key presses are not delivered to HUD, other UI
		/// elements and the game until the display is discarded either by returning or being destroyed.
		void add_custom_blocking_ui_panel(RectF position_ratio, std::variant<const ui::PanelBlueprint*, std::unique_ptr<ui::PanelBlueprint>> blueprint);
		void remove_custom_blocking_ui_panel(); // Should not be called from the custom UI itself
		void remove_custom_blocking_ui_panel_deferred(); // Can be called from the custom UI itself

		/// In panning mode, mouse states are not cleared by UI elements so that panning the map is possible even
		/// thought the mouse spills into UI elements.
		bool is_panning() const {  return _is_panning; }
		void enable_panning() { _is_panning = true; }
		void disable_panning() { _is_panning = false; }

		[[deprecated]] void display_message(const std::string& msg);
		[[deprecated]] void remove_message();

	   private:
		void_expected init_any_player(
		    const std::variant<std::filesystem::path, pb::Level>& level_path_or_blueprint, const std::string& name,
		    bool physical_world, void (m2g::Proxy::*pre_level_init)(const std::string&, const pb::Level&),
		    void (m2g::Proxy::*post_level_init)(const std::string&, const pb::Level&));

		friend class Game;
	};
}  // namespace m2
