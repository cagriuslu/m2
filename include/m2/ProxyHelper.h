#pragma once
#include <m2/Chrono.h>
#include <m2g/Proxy.h>
#include <m2/Meta.h>

namespace m2g {
	constexpr auto LockstepGameTickFrequencyF = m2::ToFloat(
		std::chrono::duration_cast<decltype(Proxy::lockstepGameTickPeriod)>(std::chrono::seconds{1}).count())
			/ m2::ToFloat(Proxy::lockstepGameTickPeriod.count());

	constexpr auto LockstepPhysicsSimulationCountPerGameTick = std::chrono::duration_cast<m2::Stopwatch::Duration>(Proxy::lockstepGameTickPeriod).count()
		/ m2::TIME_BETWEEN_PHYSICS_SIMULATIONS.count();
	static_assert((std::chrono::duration_cast<m2::Stopwatch::Duration>(Proxy::lockstepGameTickPeriod).count() % m2::TIME_BETWEEN_PHYSICS_SIMULATIONS.count()) == 0,
		"Lockstep game tick period isn't an integer multiple of time between physics simulations");
}
