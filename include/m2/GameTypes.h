#pragma once
#include <m2g/Proxy.h>
#include <m2/math/primitives/Fixed.h>
#include <m2/math/primitives/Float.h>
#include <type_traits>

namespace m2 {
	using F = std::conditional_t<m2g::Proxy::deterministic, Fixed, Float>;
}
