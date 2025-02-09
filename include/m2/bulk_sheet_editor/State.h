#pragma once
#include <filesystem>

#include "../Meta.h"

namespace m2::bulk_sheet_editor {
	class State {
		std::filesystem::path _sprite_sheets_path;
		std::string _selected_resource;
		sdl::TextureUniquePtr _texture;
		VecI _textureDimensions;
		int _ppm{};
		std::optional<RectI> _savedSpriteRect;

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

		// Modifiers

		std::optional<pb::SpriteSheet> SelectResource(const std::string& resource);
		void LookUpAndStoreSpriteRect(std::optional<m2g::pb::SpriteType>);
		void SetRect(m2g::pb::SpriteType, const RectI&);
		void Reset(m2g::pb::SpriteType sprite);

		void Draw() const;
	};
}  // namespace m2::bulk_sheet_editor
