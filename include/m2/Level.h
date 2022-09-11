#ifndef M2_LEVEL_H
#define M2_LEVEL_H

#include "Object.h"
#include "Value.h"
#include <LevelBlueprint.pb.h>
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
		pb::LevelBlueprint _lb;

		Level(Type type, std::string lb_path);

	public:
		[[nodiscard]] Type type() const;

		std::vector<std::function<void(void)>> deferred_actions;

		// Editor
		std::optional<std::string> editor_file_path;
		enum class EditorMode {
			NONE,
			PAINT
		} editor_mode{};
		void activate_mode(EditorMode mode);
		void editor_paint_mode_select_sprite(int index);
		int editor_paint_mode_selected_sprite{-1};
		ID editor_paint_mode_selected_sprite_ghost_id{0};

		static Value<Level> create_single_player_level(const std::string& lb_path);
		static Value<Level> create_editor_level(const std::string& lb_path);
	};
}

#endif //M2_LEVEL_H
