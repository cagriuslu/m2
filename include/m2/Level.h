#ifndef M2_LEVEL_H
#define M2_LEVEL_H

#include "Object.h"
#include "Value.h"
#include "Vec2i.h"
#include <Level.pb.h>
#include <functional>
#include <optional>
#include <string>

namespace m2 {
	class Level {
	public:
		enum class Type {
			SINGLE_PLAYER,
			EDITOR
		};

	private:
		Type _type;
		std::string _lb_path;
		pb::Level _lb;

		Level(Type type, std::string lb_path);

	public:
		[[nodiscard]] Type type() const;

		std::vector<std::function<void(void)>> deferred_actions;

		// Editor
		enum class EditorMode {
			NONE,
			PAINT,
			ERASE,
			PLACE,
			REMOVE
		} editor_mode{};
		void activate_mode(EditorMode mode);
		m2g::pb::SpriteType editor_paint_mode_selected_sprite_type{};
		void editor_paint_mode_select_sprite_type(m2g::pb::SpriteType sprite_type);
		void editor_paint_mode_paint_sprite(const Vec2i& position);
		void editor_erase_mode_erase_position(const Vec2i& position);
		m2g::pb::ObjectType editor_place_mode_selected_object_type{};
		void editor_place_mode_select_object_type(m2g::pb::ObjectType object_type);
		void editor_place_mode_place_object(const Vec2i& position);
		void editor_remove_mode_remove_object(const Vec2i& position);
		Id editor_paint_or_place_mode_selected_sprite_ghost_id{};
		std::unordered_map<Vec2i, Id, Vec2iHash> editor_bg_placeholders;
		std::unordered_map<Vec2i, Id, Vec2iHash> editor_fg_placeholders;

		static Value<Level> create_single_player_level(const std::string& lb_path);
		static Value<Level> create_editor_level(const std::string& lb_path);
	};
}

#endif //M2_LEVEL_H
