#pragma once
#include <m2g/Proxy.h>
#include <m2/Meta.h>

namespace m2g {
	constexpr auto LockstepGameTickFrequencyF = m2::ToFloat(
		std::chrono::duration_cast<decltype(Proxy::lockstepGameTickPeriod)>(std::chrono::seconds{1}).count())
			/ m2::ToFloat(Proxy::lockstepGameTickPeriod.count());
}
