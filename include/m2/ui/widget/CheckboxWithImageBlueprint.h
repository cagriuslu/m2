#pragma once
#include "../UiAction.h"
#include <m2g_SpriteType.pb.h>
#include <m2g_KeyType.pb.h>
#include <functional>
#include <optional>
#include <utility>
#include <variant>

namespace m2::widget {
	// Forward declaration
	class CheckboxWithImage;

	struct CheckboxWithImageBlueprint {
		// Either a sprite to display, or a callback that draws the checkbox content
		std::variant<m2g::pb::SpriteType, std::function<void(const CheckboxWithImage&)>> content{};
		bool initial_state{};
		m2g::pb::KeyType keyboardShortcut{};

		std::function<void(CheckboxWithImage& self)> onCreate{};
		std::function<void(CheckboxWithImage& self)> onHover{};
		std::function<void(CheckboxWithImage& self)> offHover{};
		std::function<std::pair<UiAction, std::optional<m2g::pb::SpriteType>>(const CheckboxWithImage& self)> onUpdate{};
		std::function<UiAction(CheckboxWithImage& self)> onAction{};
	};
}
