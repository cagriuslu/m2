#include <gtest/gtest.h>
#include <m2/common/SpinLock.h>

// SpinLock only exposes lock()/unlock() (no try-lock). Exercise the
// single-threaded deterministic path: acquire, release, and re-acquire.
TEST(SpinLock, LockThenUnlock) {
	m2::SpinLock lock;
	lock.lock();
	lock.unlock();
	SUCCEED();
}

TEST(SpinLock, CanReacquireAfterUnlock) {
	m2::SpinLock lock;
	lock.lock();
	lock.unlock();
	// A fresh acquisition must succeed once the lock has been released.
	lock.lock();
	lock.unlock();
	SUCCEED();
}
