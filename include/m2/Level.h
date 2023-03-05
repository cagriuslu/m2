#ifndef M2_LEVEL_H
#define M2_LEVEL_H

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
			LEVEL_EDITOR
		};

	private:
		Type _type{};
		std::optional<FilePath> _lb_path;
		std::optional<pb::Level> _lb;

	public:
		~Level();

		[[nodiscard]] inline Type type() const { return _type; }

		// Objects are not meant to be iterated over, as it holds all types of objects. If a certain type of object needs to
		// be iterated over, create a component, attach the component to an object, put component in separate pool, and
		// iterate over that pool.
		// Another reason to put a component inside a Pool: if the type of object that is using that component is
		// created/destroyed very rapidly.
		Pool<Object> objects;
		std::unordered_map<GroupId, std::unique_ptr<Group>, GroupId::Hash> groups;
		DrawList draw_list;
		Pool<Physique> physics;
		Pool<Graphic> graphics;
		Pool<Graphic> terrainGraphics;
		Pool<Light> lights;
		Pool<Sound> sounds;
		Pool<CharacterVariant> characters;
		b2World *world{};
		box2d::ContactListener* contactListener{};
		Id cameraId{}, playerId{}, pointerId{};
		std::optional<SoundListener> left_listener, right_listener;
		std::optional<Pathfinder> pathfinder;
		std::optional<ui::State> leftHudUIState, rightHudUIState;
		std::vector<std::function<void(void)>> deferred_actions;
		VoidValue init_single_player(const std::variant<FilePath,pb::Level>& level_path_or_blueprint);

		struct LevelEditorState {
			struct PaintMode {
				m2g::pb::SpriteType selected_sprite_type{};
				Id selected_sprite_ghost_id{};
				void select_sprite_type(m2g::pb::SpriteType sprite_type);
				void paint_sprite(const Vec2i& position);
			};
			struct EraseMode {
				void erase_position(const Vec2i& position);
			};
			struct PlaceMode {
				Id selected_sprite_ghost_id{};
				m2g::pb::ObjectType selected_object_type{};
				m2g::pb::GroupType selected_group_type{};
				unsigned selected_group_instance{};
				void select_object_type(m2g::pb::ObjectType object_type);
				void select_group_type(m2g::pb::GroupType group_type);
				void select_group_instance(unsigned group_instance);
				void place_object(const Vec2i& position);
			};
			struct RemoveMode {
				void remove_object(const Vec2i& position);
			};

			std::variant<std::monostate,PaintMode,EraseMode,PlaceMode,RemoveMode> mode;
			std::unordered_map<Vec2i, Id, Vec2iHash> bg_placeholders;
			std::unordered_map<Vec2i, Id, Vec2iHash> fg_placeholders;

			void deactivate_mode();
			void activate_paint_mode();
			void activate_erase_mode();
			void activate_place_mode();
			void activate_remove_mode();
			static void save();
		};
		std::optional<LevelEditorState> level_editor_state;
		VoidValue init_level_editor(const FilePath& lb_path);

		// Accessors
		inline Object* player() { return objects.get(playerId); }
	};
}

#endif //M2_LEVEL_H
