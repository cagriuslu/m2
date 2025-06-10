#pragma  once
#include "../math/VecF.h"

namespace m2 {
	struct CircF {
		VecF center;
		float r{};

		CircF() = default;
		CircF(const VecF& center, float r) : center(center), r(r) {}
	};
}
