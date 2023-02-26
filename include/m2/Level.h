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
			EDITOR
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
		Pool<CharacterVariant> characters;
		b2World *world{};
		box2d::ContactListener* contactListener{};
		Id cameraId{}, playerId{}, pointerId{};
		std::optional<Pathfinder> pathfinder;
		std::optional<ui::State> leftHudUIState, rightHudUIState;
		std::vector<std::function<void(void)>> deferred_actions;

		VoidValue init_single_player(const std::variant<FilePath,pb::Level>& level_path_or_blueprint);
		VoidValue init_editor(const FilePath& lb_path);

		// Editor
		enum class EditorMode {
			NONE,
			PAINT,
			ERASE,
			PLACE,
			REMOVE
		} editor_mode{};
		void activate_mode(EditorMode mode);
		// Editor members
		std::unordered_map<Vec2i, Id, Vec2iHash> editor_bg_placeholders;
		std::unordered_map<Vec2i, Id, Vec2iHash> editor_fg_placeholders;
		m2g::pb::SpriteType editor_paint_mode_selected_sprite_type{};
		// Paint mode
		Id editor_paint_or_place_mode_selected_sprite_ghost_id{};
		void editor_paint_mode_select_sprite_type(m2g::pb::SpriteType sprite_type);
		void editor_paint_mode_paint_sprite(const Vec2i& position);
		// Erase mode
		void editor_erase_mode_erase_position(const Vec2i& position);
		// Place mode
		m2g::pb::ObjectType editor_place_mode_selected_object_type{};
		m2g::pb::GroupType editor_place_mode_selected_group_type{};
		unsigned editor_place_mode_selected_group_instance{};
		void editor_place_mode_select_object_type(m2g::pb::ObjectType object_type);
		void editor_place_mode_select_group_type(m2g::pb::GroupType group_type);
		void editor_place_mode_select_group_instance(unsigned group_instance);
		void editor_place_mode_place_object(const Vec2i& position);
		// Remove mode
		void editor_remove_mode_remove_object(const Vec2i& position);
		// Save
		void editor_save();

		// Accessors
		inline Object* player() { return objects.get(playerId); }
	};
}

#endif //M2_LEVEL_H
