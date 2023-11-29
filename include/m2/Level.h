#pragma once
#include "level_editor/Detail.h"
#include "level_editor/State.h"
#include "pixel_editor/State.h"
#include "sheet_editor/State.h"
#include "sheet_editor/DynamicImageLoader.h"
#include <m2/game/DynamicGridLinesLoader.h>
#include "Object.h"
#include "ui/State.h"
#include "DrawList.h"
#include "Pathfinder.h"
#include "Meta.h"
#include <Level.pb.h>
#include "single_player/State.h"
#include <box2d/b2_world.h>
#include <functional>
#include <optional>
#include <string>

namespace m2 {
	class Level final {
		std::optional<std::filesystem::path> _lb_path;
		std::optional<pb::Level> _lb;
		std::string _name;
		pb::ProjectionType _projection_type{pb::PARALLEL_TOP};

	public:
		~Level();

		// Objects are not meant to be iterated over, as it holds all types of objects. If a certain type of object needs to
		// be iterated over, create a component, attach the component to an object, put component in separate pool, and
		// iterate over that pool.
		// Another reason to put a component inside a Pool: if the type of object that is using that component is
		// created/destroyed very rapidly.
		Pool<Object> objects;
		std::map<GroupId, std::unique_ptr<Group>, GroupId::Less> groups;
		DrawList draw_list;
		Pool<Physique> physics;
		Pool<Graphic> graphics;
		std::array<Pool<Graphic>, static_cast<int>(BackgroundLayer::n)> terrain_graphics; // First pool is the front-most terrain
		Pool<Light> lights;
		Pool<SoundEmitter> sound_emitters;
		Pool<CharacterVariant> characters;
		b2World *world{};
		box2d::ContactListener* contact_listener{};
		Id camera_id{}, player_id{}, pointer_id{};
		std::optional<SoundListener> left_listener, right_listener;
		std::optional<Pathfinder> pathfinder;
		std::optional<ui::State> left_hud_ui_state, right_hud_ui_state;
		std::optional<std::string> message;
		std::optional<ui::State> message_box_ui_state;
		std::optional<sdl::ticks_t> level_start_ticks;
		std::optional<sdl::ticks_t> level_start_pause_ticks;
		std::vector<std::function<void()>> deferred_actions;
		std::variant<std::monostate, splayer::State, ledit::State, pedit::State, sedit::State> type_state;
		std::optional<DynamicGridLinesLoader> dynamic_grid_lines_loader;
		std::optional<DynamicGridLinesLoader> dynamic_sheet_grid_lines_loader;

		void_expected init_single_player(const std::variant<std::filesystem::path,pb::Level>& level_path_or_blueprint, const std::string& name);
		void_expected init_level_editor(const std::filesystem::path& lb_path);
		void_expected init_pixel_editor(const std::filesystem::path& path, int x_offset, int y_offset);
		void_expected init_sheet_editor(const std::filesystem::path& path);
		void_expected reset_sheet_editor();

		// Accessors
		std::optional<std::filesystem::path> path() const { return _lb_path; }
		std::optional<pb::Level> level_blueprint() const { return _lb; }
		const std::string& name() const { return _name; }
		const std::string& identifier() const { return _lb->identifier(); }
		pb::ProjectionType projection_type() const { return _projection_type; }
		Object* player() { return objects.get(player_id); }
		Object* camera() { return objects.get(camera_id); }
		sdl::ticks_t get_level_duration() const;

		// Modifiers
		void begin_game_loop();
		void display_message(const std::string& msg, float timeout = 5.0f);
	};
}
