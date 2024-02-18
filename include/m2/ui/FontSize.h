#pragma once
#include <variant>

namespace m2::ui {
	struct FixedSize {
		float size;
	};
	struct FixedRatio {
		float ratio;
	};
	/// std::monostate -> The font is drawn such that the container is filled with the text.
	/// FixedSize -> The font has a fixed size regardless of the size of the container.
	/// FixedRatio -> The font has a fixed ration with respect to the size of the container.
	using FontSize = std::variant<std::monostate, FixedSize, FixedRatio>;
}
