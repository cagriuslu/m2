#pragma once
#include <Sprite.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/protobuf/PersistentObject.h>
#include <filesystem>

#include "../math/CircF.h"
#include "../Meta.h"
#include "../ui/UiPanel.h"

namespace m2::sheet_editor {
	struct State {
		pb::PersistentObject<pb::SpriteSheets> _persistentSpriteSheets;

		m2g::pb::SpriteType _selected_sprite_type{};
		sdl::TextureUniquePtr _texture;
		VecI _textureDimensions;
		int _ppm{};

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
		std::variant<std::monostate, ForegroundCompanionMode, BackgroundColliderMode, ForegroundColliderMode>
		    mode;

		static m2::expected<State> create(const std::filesystem::path& path);

		// Accessors

		const pb::SpriteSheets& SpriteSheets() const { return _persistentSpriteSheets.Cache(); }
		const pb::Sprite& selected_sprite() const;
		void modify_selected_sprite(const std::function<void(pb::Sprite&)>& modifier);  // This function re-reads the file every time it's called.
		RectI selected_sprite_rect() const;  // This function re-reads the file every time it's called. // TODO is it necessary
		VecF selected_sprite_center() const;  // This function re-reads the file every time it's called. // TODO is it necessary
		VecF selected_sprite_origin() const;  // This function re-reads the file every time it's called. // TODO is it necessary

		// To be used by the main menu

		void Select(m2g::pb::SpriteType);

		// To be used by left hud

		void deactivate_mode();
		void activate_foreground_companion_mode();
		void activate_background_collider_mode();
		void activate_foreground_collider_mode();

		// Modifiers

		void SetSpriteRect(const RectI& rect);
		void SetSpriteOrigin(const VecF& origin);
		void ResetSpriteRectAndOrigin();
		void AddForegroundCompanionRect(const RectI& rect);
		void SetForegroundCompanionCenter(const VecF& center);
		void RemoveForegroundCompanion();
		void AddRectangleBackgroundCollider(const RectF& rect);
		void AddCircleBackgroundCollider(const VecF& center, float radius);
		void RemoveBackgroundColliders();
		void AddRectangleForegroundCollider(const RectF& rect);
		void AddCircleForegroundCollider(const VecF& center, float radius);
		void RemoveForegroundColliders();

		void Draw() const;

	   private:
		explicit State(pb::PersistentObject<pb::SpriteSheets>&& persistentSpriteSheets)
				: _persistentSpriteSheets(std::move(persistentSpriteSheets)) {}
	};

	void modify_sprite_in_sheet(
	    const std::filesystem::path& path, m2g::pb::SpriteType type, const std::function<void(pb::Sprite&)>& modifier); // TODO remove and use ModifySpriteInSheets
	void ModifySpriteInSheets(pb::PersistentObject<pb::SpriteSheets>& persistentObject, m2g::pb::SpriteType spriteType,
			const std::function<void(pb::Sprite&)>& modifier);
}  // namespace m2::sheet_editor
