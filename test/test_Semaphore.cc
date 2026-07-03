#include <gtest/gtest.h>
#include <m2/common/Semaphore.h>

// down() only blocks when the internal count reaches zero. These tests stay on
// the non-blocking counting path: never acquire more than has been signalled.

TEST(Semaphore, SignalThenAcquireSameCount) {
	m2::Semaphore sem{0};
	sem.up(5);
	// 5 units available, so down(5) completes without blocking.
	sem.down(5);
	SUCCEED();
}

TEST(Semaphore, AcquireInSmallerChunks) {
	m2::Semaphore sem{0};
	sem.up(4);
	sem.down(1);
	sem.down(3);
	SUCCEED();
}

TEST(Semaphore, IncrementalSignalsAccumulate) {
	m2::Semaphore sem{0};
	sem.up(1);
	sem.up(1);
	sem.up(1);
	// Three separate up()s accumulate to 3, so a single down(3) completes.
	sem.down(3);
	SUCCEED();
}

TEST(Semaphore, DefaultInitialStateAllowsOneAcquire) {
	m2::Semaphore sem; // default initial_state == 1
	sem.down(1);
	SUCCEED();
}
