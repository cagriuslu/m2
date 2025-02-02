#pragma once
#include <Sprite.pb.h>
#include <m2g_SpriteType.pb.h>

#include <filesystem>

#include "../math/CircF.h"
#include "../Meta.h"
#include "../ui/UiPanel.h"

namespace m2::sheet_editor {
	struct State {
		const std::filesystem::path _path;
		mutable pb::SpriteSheets _sprite_sheets;
		m2g::pb::SpriteType _selected_sprite_type{};
		sdl::TextureUniquePtr _texture;
		VecI _textureDimensions;
		int _ppm;

		struct ForegroundCompanionMode {
			// Secondary mouse button sets the center
			std::optional<m2::VecF> secondary_selection_position;

			ForegroundCompanionMode();
			// Disable copy, default move
			ForegroundCompanionMode(const ForegroundCompanionMode& other) = delete;
			ForegroundCompanionMode& operator=(const ForegroundCompanionMode& other) = delete;
			ForegroundCompanionMode(ForegroundCompanionMode&& other) = default;
			ForegroundCompanionMode& operator=(ForegroundCompanionMode&& other) = default;
			~ForegroundCompanionMode();
			void on_draw() const;

			void add_rect();
			std::vector<m2::RectI> current_rects;
			void set_center();
			std::optional<m2::VecF> current_center;
			void reset();
		};
		struct RectMode {
			// Secondary mouse button sets the center
			std::optional<m2::VecF> secondary_selection_position;

			RectMode();
			// Disable copy, default move
			RectMode(const RectMode& other) = delete;
			RectMode& operator=(const RectMode& other) = delete;
			RectMode(RectMode&& other) = default;
			RectMode& operator=(RectMode&& other) = default;
			~RectMode();
			void on_draw() const;

			void set_rect();
			std::optional<m2::RectI> current_rect;
			void set_center();
			std::optional<m2::VecF> current_center;
			void reset();
		};
		struct BackgroundColliderMode {
			BackgroundColliderMode();
			// Disable copy, default move
			BackgroundColliderMode(const BackgroundColliderMode& other) = delete;
			BackgroundColliderMode& operator=(const BackgroundColliderMode& other) = delete;
			BackgroundColliderMode(BackgroundColliderMode&& other) = default;
			BackgroundColliderMode& operator=(BackgroundColliderMode&& other) = default;
			~BackgroundColliderMode();
			void on_draw() const;

			void set();
			std::optional<m2::RectF> current_rect;  // wrt sprite center
			std::optional<CircF> current_circ;  // wrt sprite center
			void reset();
		};
		struct ForegroundColliderMode {
			ForegroundColliderMode();
			// Disable copy, default move
			ForegroundColliderMode(const ForegroundColliderMode& other) = delete;
			ForegroundColliderMode& operator=(const ForegroundColliderMode& other) = delete;
			ForegroundColliderMode(ForegroundColliderMode&& other) = default;
			ForegroundColliderMode& operator=(ForegroundColliderMode&& other) = default;
			~ForegroundColliderMode();
			void on_draw() const;

			void set();
			std::optional<m2::RectF> current_rect;  // wrt sprite center
			std::optional<CircF> current_circ;  // wrt sprite center
			void reset();
		};
		std::variant<std::monostate, ForegroundCompanionMode, RectMode, BackgroundColliderMode, ForegroundColliderMode>
		    mode;

		static m2::expected<State> create(const std::filesystem::path& path);

		const pb::SpriteSheets& sprite_sheets() const;  // This function re-reads the file every time it's called.
		const pb::Sprite& selected_sprite() const;  // This function re-reads the file every time it's called.
		void modify_selected_sprite(const std::function<void(pb::Sprite&)>&
		                                modifier);  // This function re-reads the file every time it's called.
		RectI selected_sprite_rect() const;  // This function re-reads the file every time it's called.
		VecF selected_sprite_center() const;  // This function re-reads the file every time it's called.
		VecF selected_sprite_origin() const;  // This function re-reads the file every time it's called.

		// To be used by the main menu
		void set_sprite_type(m2g::pb::SpriteType);
		void select();

		// To be used by left hud
		void deactivate_mode();
		void activate_foreground_companion_mode();
		void activate_rect_mode();
		void activate_background_collider_mode();
		void activate_foreground_collider_mode();

		void Draw() const;

	   private:
		inline explicit State(std::filesystem::path path) : _path(std::move(path)) {}
	};

	void modify_sprite_in_sheet(
	    const std::filesystem::path& path, m2g::pb::SpriteType type, const std::function<void(pb::Sprite&)>& modifier);
}  // namespace m2::sheet_editor
