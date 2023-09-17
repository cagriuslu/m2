#pragma  once
#include "VecF.h"

namespace m2 {
	struct CircF {
		VecF center;
		float r{};

		CircF() = default;
		inline CircF(const VecF& center, float r) : center(center), r(r) {}
	};
}
