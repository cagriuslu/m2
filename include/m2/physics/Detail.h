#pragma once
// #include <m2/common/math/Exact.h>
#include <m2/common/math/Float.h>

namespace m2 {
	// using PhysicsPrimitive = Exact;
	using PhysicsPrimitive = Float;

	const auto physicsDeltaTime = PhysicsPrimitive{0.01f};
	const auto physicsDeltaTimeSquared = PhysicsPrimitive{0.001f};
	const auto physicsDefaultVelocitySustainment = PhysicsPrimitive{0.95f};
	const auto physicsDefaultAngularVelocitySustainment = PhysicsPrimitive{0.95f};
}
