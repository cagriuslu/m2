#pragma once
#include <cstdint>

namespace m2::thirdparty::video {
	using Ticks = int64_t;
	void Delay(Ticks duration);
	Ticks GetTicks();
	Ticks GetTicksSince(Ticks lastTicks, Ticks pauseTicks = 0);

	int GetRefreshRate();

	void InitAll();
	void DeinitAll();
}
