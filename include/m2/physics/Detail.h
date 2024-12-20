#pragma once
// #include <m2/math/primitives/Fixed.h>
#include <m2/math/primitives/Float.h>

namespace m2 {
	// using PhysicsPrimitive = Fixed;
	using PhysicsPrimitive = Float;

	const auto physicsDeltaTime = PhysicsPrimitive{0.01f};
	const auto physicsDeltaTimeSquared = PhysicsPrimitive{0.001f};
	const auto physicsDefaultDamping = PhysicsPrimitive{0.95f};
}
