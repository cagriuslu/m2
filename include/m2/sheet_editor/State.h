#pragma once
#include "../Meta.h"
#include "../ui/Blueprint.h"
#include "../ui/State.h"
#include <m2g_SpriteType.pb.h>
#include <Sprite.pb.h>
#include <filesystem>
#include <utility>

namespace m2::sedit {
	class State {
		const std::filesystem::path _path;
		mutable pb::SpriteSheets _sprite_sheets;
		m2g::pb::SpriteType _selected_sprite_type{};

		inline explicit State(std::filesystem::path path) : _path(std::move(path)) {}

	public:
		static m2::expected<State> create(const std::filesystem::path& path);

		// This function re-reads the file every time it's called.
		const pb::SpriteSheets& sprite_sheets() const;

		void select_sprite_type(m2g::pb::SpriteType);
	};
}
