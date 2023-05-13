#ifndef M2_LEVEL_H
#define M2_LEVEL_H

#include "level_editor/Detail.h"
#include "Object.h"
#include "Ui.h"
#include "DrawList.h"
#include "Pathfinder.h"
#include "Value.h"
#include "Vec2i.h"
#include <Level.pb.h>
#include <functional>
#include <optional>
#include <string>

namespace m2 {
	class Level final {
	public:
		enum class Type {
			NO_TYPE,
			SINGLE_PLAYER,
			LEVEL_EDITOR,
			PIXEL_EDITOR
		};

	private:
		Type _type{};
		std::optional<std::filesystem::path> _lb_path;
		std::optional<pb::Level> _lb;
		std::string _name;

	public:
		~Level();

		[[nodiscard]] inline Type type() const { return _type; }

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
		Pool<Graphic> terrain_graphics;
		Pool<Light> lights;
		Pool<SoundEmitter> sound_emitters;
		Pool<CharacterVariant> characters;
		b2World *world{};
		box2d::ContactListener* contact_listener{};
		Id camera_id{}, player_id{}, pointer_id{};
		std::optional<SoundListener> left_listener, right_listener;
		std::optional<Pathfinder> pathfinder;
		std::optional<ui::State> left_hud_ui_state, right_hud_ui_state;
		std::optional<sdl::ticks_t> level_start_ticks;
		std::optional<sdl::ticks_t> level_start_pause_ticks;
		std::vector<std::function<void(void)>> deferred_actions;

		VoidValue init_single_player(const std::variant<std::filesystem::path,pb::Level>& level_path_or_blueprint, const std::string& name);

		struct LevelEditorState {
			struct PaintMode {
				m2g::pb::SpriteType selected_sprite_type{};
				Id selected_sprite_ghost_id{};
				void select_sprite_type(m2g::pb::SpriteType sprite_type);
				void paint_sprite(const Vec2i& position);

				~PaintMode();
			};
			struct EraseMode {
				static void erase_position(const Vec2i& position);
			};
			struct PlaceMode {
				Id selected_sprite_ghost_id{};
				m2g::pb::ObjectType selected_object_type{};
				m2g::pb::GroupType selected_group_type{};
				unsigned selected_group_instance{};
				void select_object_type(m2g::pb::ObjectType object_type);
				void select_group_type(m2g::pb::GroupType group_type);
				void select_group_instance(unsigned group_instance);
				void place_object(const Vec2i& position) const;

				~PlaceMode();
			};
			struct RemoveMode {
				static void remove_object(const Vec2i& position);
			};
			struct ShiftMode {
				enum class ShiftType {
					RIGHT,
					DOWN,
					RIGHT_N_DOWN
				} shift_type;
				void shift(const Vec2i& position) const;
			};

			std::variant<std::monostate,PaintMode,EraseMode,PlaceMode,RemoveMode,ShiftMode> mode;
			level_editor::BackgroundPlaceholderMap bg_placeholders; // TODO Map2i might be used instead
			level_editor::ForegroundPlaceholderMap fg_placeholders; // TODO Map2i might be used instead

			void deactivate_mode();
			void activate_paint_mode();
			void activate_erase_mode();
			void activate_place_mode();
			void activate_remove_mode();
			void activate_shift_mode();
			static void save();
		};
		std::optional<LevelEditorState> level_editor_state;
		VoidValue init_level_editor(const std::filesystem::path& lb_path);

		struct PixelEditorState {
			struct PaintMode {
				static void paint_color(const Vec2i& position);
			};
			struct EraseMode {
				static void erase_color(const Vec2i& position);
			};
			struct ColorPickerMode {
				static void pick_color(const Vec2i& position);
			};

			std::variant<std::monostate,PaintMode,EraseMode,ColorPickerMode> mode;
			SDL_Color selected_color;
			std::unordered_map<Vec2i, std::pair<Id,SDL_Color>, Vec2iHash> pixels;
			sdl::SurfaceUniquePtr image_surface;
			Vec2i image_offset;

			inline void select_color(const SDL_Color& color) { selected_color = color; }
			void deactivate_mode();
			void activate_paint_mode();
			void activate_erase_mode();
			void activate_color_picker_mode();
			static void save();
		};
		std::optional<PixelEditorState> pixel_editor_state;
		VoidValue init_pixel_editor(const std::filesystem::path& path, int x_offset, int y_offset);

		void begin_game_loop();

		// Accessors
		inline std::optional<std::filesystem::path> path() const { return _lb_path; }
		inline const std::string& name() const { return _name; }
		inline Object* player() { return objects.get(player_id); }
		inline Object* camera() { return objects.get(camera_id); }
		sdl::ticks_t get_level_duration() const;

		// Convenience
		void toggle_grid();
	};
}

#endif //M2_LEVEL_H
