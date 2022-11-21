#include <m2/SpinLock.h>

void m2::SpinLock::lock() {
	for (;;) {
		if (not _lock.exchange(true, std::memory_order_acquire)) {
			break;
		}
		while (_lock.load(std::memory_order_relaxed)) {
#if defined(_MSC_VER)
			_mm_pause();
#elif defined(__x86_64__)
            __builtin_ia32_pause();
#elif defined(__arm64__)
            asm volatile("yield");
#endif
		}
	}
}

void m2::SpinLock::unlock() {
	_lock.store(false, std::memory_order_release);
}
