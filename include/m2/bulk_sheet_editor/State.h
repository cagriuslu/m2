#pragma once
#include <filesystem>

#include "../Meta.h"

namespace m2::bulk_sheet_editor {
	class State {
		std::filesystem::path _sprite_sheets_path;
		std::pair<std::string, int> _selected_resource;  // and ppm
		std::pair<m2g::pb::SpriteType, RectI> _selected_sprite; // TODO get from HUD
		sdl::TextureUniquePtr _texture;
		VecI _textureDimensions;
		int _ppm;

		explicit State(std::filesystem::path sprite_sheets_path) : _sprite_sheets_path(std::move(sprite_sheets_path)) {}

	public:
		static expected<State> Create(const std::filesystem::path& sprite_sheets_path);
		State(const State& other) = delete;
		State& operator=(const State& other) = delete;
		State(State&& other) = default;
		State& operator=(State&& other) = default;
		~State();

		// Accessors

		[[nodiscard]] pb::SpriteSheets ReadSpriteSheetsFromFile() const;
		[[nodiscard]] std::optional<pb::SpriteSheet> ReadSelectedSpriteSheetFromFile() const;

		// Modifiers

		bool SelectSpriteSheetResource(const std::string& resource);
		void SelectSpriteType(m2g::pb::SpriteType type); // TODO get from HUD
		void ModifySelectedSprite(const std::function<void(pb::Sprite&)>& modifier) const;
		void set_rect();
		void reset();

		void Draw() const;

	};
}  // namespace m2::bulk_sheet_editor
