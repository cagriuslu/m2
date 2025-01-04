#pragma once
#include "../ui/UiAction.h"
#include "Detail.h"

namespace m2::ledit {
	struct State {
		struct PaintMode {
			m2g::pb::SpriteType selected_sprite_type{};
			Id selected_sprite_ghost_id{};

			void select_sprite_type(m2g::pb::SpriteType sprite_type);
			void paint_sprite(const VecI& position);
			~PaintMode();
		};
		struct EraseMode {
			void erase_position(const VecI& position);
			static void erase_position(const VecI& position, BackgroundLayer layer);
		};
		struct PlaceMode {
			Id selected_sprite_ghost_id{};
			m2g::pb::ObjectType selected_object_type{};
			m2g::pb::GroupType selected_group_type{};
			unsigned selected_group_instance{};
			void select_object_type(m2g::pb::ObjectType object_type);
			void select_group_type(m2g::pb::GroupType group_type);
			void select_group_instance(unsigned group_instance);
			void place_object(const VecI& position) const;

			~PlaceMode();
		};
		struct RemoveMode {
			static void remove_object(const VecI& position);
		};
		struct PickMode {
			bool pick_foreground{};

			std::optional<m2g::pb::SpriteType> lookup_background_sprite(const VecI& position);
			std::optional<m2::pb::LevelObject> lookup_foreground_object(const VecI& position);
		};
		struct SelectMode {
			std::optional<BackgroundLayer> clipboard_layer;
			std::optional<m2::VecI> clipboard_position_1, clipboard_position_2;  // TopLeft, BottomRight

			SelectMode();
			// Disable copy, default move
			SelectMode(const SelectMode& other) = delete;
			SelectMode& operator=(const SelectMode& other) = delete;
			SelectMode(SelectMode&& other) = default;
			SelectMode& operator=(SelectMode&& other) = default;
			~SelectMode();
			void on_draw() const;

			void shift_right();
			void shift_down();
			void copy();
			void paste_bg();
			void paste_fg();
			void erase();
			void remove();
			std::vector<m2g::pb::SpriteType> rfill_sprite_types;  // Filled by UI
			UiAction rfill();
		};
		struct ShiftMode {
			enum class ShiftType { RIGHT, DOWN, RIGHT_N_DOWN } shift_type;

			void shift(const VecI& position) const;
		};

		std::variant<std::monostate, PaintMode, EraseMode, PlaceMode, RemoveMode, PickMode, SelectMode, ShiftMode> mode;
		std::array<level_editor::BackgroundPlaceholderMap, static_cast<int>(BackgroundLayer::n)>
		    bg_placeholders;  // TODO Map2i might be used instead
		level_editor::ForegroundPlaceholderMap fg_placeholders;  // TODO Map2i might be used instead
		BackgroundLayer selected_layer{};

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
}  // namespace m2::ledit
