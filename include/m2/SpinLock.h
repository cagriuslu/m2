#ifndef M2_SPINLOCK_H
#define M2_SPINLOCK_H

#include <atomic>

namespace m2 {
	class SpinLock {
		std::atomic<bool> _lock{false};
	public:
		void lock();
		void unlock();
	};
}

#endif //M2_SPINLOCK_H
