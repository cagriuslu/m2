#pragma once
#include "../Meta.h"
#include "../ui/Blueprint.h"
#include "../ui/State.h"
#include "../sheet_editor/DynamicImageLoader.h"
#include <m2g_SpriteType.pb.h>
#include <Sprite.pb.h>
#include <filesystem>
#include <utility>

namespace m2::sedit {
	struct State {
		const std::filesystem::path _path;
		mutable pb::SpriteSheets _sprite_sheets;
		m2g::pb::SpriteType _selected_sprite_type{};

		struct ForegroundCompanionMode {
			// For setting the rectangle
			std::optional<m2::VecI> primary_selection_position_1, primary_selection_position_2; // TopLeft, BottomRight
			// Secondary mouse button sets the center
			std::optional<m2::VecF> secondary_selection_position;

			ForegroundCompanionMode();

			void add_rect();
			std::vector<m2::RectI> current_rects;
			void set_center();
			std::optional<m2::VecF> current_center;
			void reset();
		};
		struct RectMode {
			// For setting the rectangle
			std::optional<m2::VecI> primary_selection_position_1, primary_selection_position_2; // TopLeft, BottomRight
			// Secondary mouse button sets the center
			std::optional<m2::VecF> secondary_selection_position;

			RectMode();

			void set_rect();
			std::optional<m2::RectI> current_rect;
			void set_center();
			std::optional<m2::VecF> current_center;
			void reset();
		};
		struct BackgroundColliderMode {
			// For setting the rectangle
			std::optional<m2::VecF> primary_selection_position_1, primary_selection_position_2; // TopLeft, BottomRight
			// For setting the circle
			std::optional<m2::VecF> secondary_selection_position_1, secondary_selection_position_2; // TopLeft, BottomRight

			BackgroundColliderMode();

			void set();
			std::optional<m2::RectF> current_rect, current_circ;
			void reset();
		};
		struct ForegroundColliderMode {
			// For setting the rectangle
			std::optional<m2::VecF> primary_selection_position_1, primary_selection_position_2; // TopLeft, BottomRight
			// For setting the circle
			std::optional<m2::VecF> secondary_selection_position_1, secondary_selection_position_2; // TopLeft, BottomRight

			ForegroundColliderMode();

			void set();
			std::optional<m2::RectF> current_rect, current_circ;
			void reset();
		};
		std::variant<std::monostate, ForegroundCompanionMode, RectMode, BackgroundColliderMode, ForegroundColliderMode> mode;

		static m2::expected<State> create(const std::filesystem::path& path);

		const pb::SpriteSheets& sprite_sheets() const; // This function re-reads the file every time it's called.
		const pb::Sprite& selected_sprite() const; // This function re-reads the file every time it's called.
		RectI selected_sprite_rect() const; // This function re-reads the file every time it's called.
		VecF selected_sprite_center() const; // This function re-reads the file every time it's called.
		VecF selected_sprite_origin() const; // This function re-reads the file every time it's called.

		// To be used by the main menu
		void select_sprite_type(m2g::pb::SpriteType);
		void prepare_sprite_selection();

		// To be used by left hud
		void deactivate_mode();
		void activate_foreground_companion_mode();
		void activate_rect_mode();
		void activate_background_collider_mode();
		void activate_foreground_collider_mode();

	private:
		inline explicit State(std::filesystem::path path) : _path(std::move(path)) {}
	};
}
