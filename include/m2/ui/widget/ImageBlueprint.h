#pragma once
#include "../UiAction.h"
#include <m2g_SpriteType.pb.h>
#include <variant>

namespace m2::widget {
	// Forward declaration
	class Image;

	struct ImageBlueprint {
		// Either a sprite to display, or a callback that draws the button content
		std::variant<m2g::pb::SpriteType, std::function<void(const Image&)>> content{};
		m2g::pb::KeyType keyboardShortcut{};

		std::function<void(Image& self)> onCreate{};
		std::function<void(Image& self)> onHover{};
		std::function<void(Image& self)> offHover{};
		std::function<std::pair<UiAction,std::optional<m2g::pb::SpriteType>>(const Image& self)> onUpdate{};
		std::function<UiAction(const Image& self)> onAction{};
	};
}
