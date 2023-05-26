#pragma once
#include "Detail.h"

namespace m2::ledit {
	struct State {
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
		struct PickMode {
			bool pick_foreground{};
			static std::optional<m2g::pb::SpriteType> lookup_background_sprite(const Vec2i& position);
			static std::optional<m2::pb::LevelObject> lookup_foreground_object(const Vec2i& position);
		};
		struct SelectMode {
			std::optional<m2::Vec2i> selection_position_1, selection_position_2; // TopLeft, BottomRight
			std::optional<m2::Vec2i> clipboard_position_1, clipboard_position_2; // TopLeft, BottomRight
			void shift_right();
			void shift_down();
			void copy();
			void paste_bg();
			void paste_fg();
			void erase();
			void remove();
		};
		struct ShiftMode {
			enum class ShiftType {
				RIGHT,
				DOWN,
				RIGHT_N_DOWN
			} shift_type;
			void shift(const Vec2i& position) const;
		};

		std::variant<std::monostate,PaintMode,EraseMode,PlaceMode,RemoveMode,PickMode,SelectMode,ShiftMode> mode;
		level_editor::BackgroundPlaceholderMap bg_placeholders; // TODO Map2i might be used instead
		level_editor::ForegroundPlaceholderMap fg_placeholders; // TODO Map2i might be used instead

		void deactivate_mode();
		void activate_paint_mode();
		void activate_erase_mode();
		void activate_place_mode();
		void activate_remove_mode();
		void activate_pick_mode();
		void activate_select_mode();
		void activate_shift_mode();
		static void save();
	};
}
