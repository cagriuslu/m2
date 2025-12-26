#pragma once
#include <m2/Chrono.h>
#include <m2g/Proxy.h>
#include <m2/Meta.h>

namespace m2g {
	constexpr auto LOCKSTEP_GAME_TICK_FREQUENCY = m2::I(m2::Stopwatch::OneSecondInDuration().count() / std::chrono::duration_cast<m2::Stopwatch::Duration>(Proxy::LOCKSTEP_GAME_TICK_PERIOD).count());
	static_assert((m2::Stopwatch::OneSecondInDuration().count() % std::chrono::duration_cast<m2::Stopwatch::Duration>(Proxy::LOCKSTEP_GAME_TICK_PERIOD).count()) == 0,
		"Lockstep game tick frequency isn't a whole integer");

	constexpr auto LOCKSTEP_PHYSICS_SIMULATION_COUNT_PER_GAME_TICK = m2::I(std::chrono::duration_cast<m2::Stopwatch::Duration>(Proxy::LOCKSTEP_GAME_TICK_PERIOD).count()
		/ m2::TIME_BETWEEN_PHYSICS_SIMULATIONS.count());
	static_assert((std::chrono::duration_cast<m2::Stopwatch::Duration>(Proxy::LOCKSTEP_GAME_TICK_PERIOD).count() % m2::TIME_BETWEEN_PHYSICS_SIMULATIONS.count()) == 0,
		"Lockstep game tick period isn't an integer multiple of time between physics simulations");
}
