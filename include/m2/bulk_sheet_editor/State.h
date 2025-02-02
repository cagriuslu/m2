#pragma once
#include <filesystem>

#include "../Meta.h"

namespace m2::bulk_sheet_editor {
	struct State {
		std::filesystem::path _sprite_sheets_path;
		std::pair<std::string, int> _selected_resource;  // and ppm
		std::pair<m2g::pb::SpriteType, RectI> _selected_sprite;
		sdl::TextureUniquePtr _texture;
		VecI _textureDimensions;
		int _ppm;

		static expected<State> create(const std::filesystem::path& sprite_sheets_path);
		State(const State& other) = delete;
		State& operator=(const State& other) = delete;
		State(State&& other) = default;
		State& operator=(State&& other) = default;
		~State();

		[[nodiscard]] pb::SpriteSheets sprite_sheets() const;
		[[nodiscard]] std::optional<pb::SpriteSheet> selected_sprite_sheet() const;

		// Used by UI
		void select_resource(const std::string& resource);
		bool select();
		void select_sprite(m2g::pb::SpriteType type);
		void modify_selected_sprite(const std::function<void(pb::Sprite&)>& modifier) const;
		void set_rect();
		void reset();

		void Draw() const;

	   private:
		explicit State(std::filesystem::path sprite_sheets_path) : _sprite_sheets_path(std::move(sprite_sheets_path)) {}
	};
}  // namespace m2::bulk_sheet_editor
