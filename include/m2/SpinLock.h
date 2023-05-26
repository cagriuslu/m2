#pragma once
#include <atomic>

namespace m2 {
	class SpinLock {
		std::atomic<bool> _lock{false};
	public:
		void lock();
		void unlock();
	};
}
