#pragma once
#include <m2/Events.h>
#include <optional>

namespace m2::thirdparty::video {
	std::optional<MouseButton> SystemButtonToMouseButton(int button);
}
