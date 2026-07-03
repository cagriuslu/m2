#include <gtest/gtest.h>
#include <m2/mt/CooperativeSleep.h>

using namespace m2;

// The real implementation of this platform seam lives in src/mt/CooperativeSleep.cc, which is NOT
// compiled into the `test` target. We supply a no-op test double so the CooperativeSleep templates
// link, and so the polling loops spin against the real steady_clock without actually sleeping. The
// classes under test are the polling/timeout templates themselves, not this seam.
namespace {
	int gStepInvocationCount = 0;
}
void m2::detail::StepActorsOnceAndWaitCooperatively(int /*waitMs*/) {
	++gStepInvocationCount;
}

TEST(CooperativeSleep, UntilReturnsImmediatelyWhenPredicateAlreadyTrue) {
	gStepInvocationCount = 0;
	int predicateEvaluations = 0;
	CooperativeSleepUntil([&]() { ++predicateEvaluations; return true; });
	// The predicate is checked once, is already true, and the loop body never runs.
	EXPECT_EQ(predicateEvaluations, 1);
	EXPECT_EQ(gStepInvocationCount, 0);
}

TEST(CooperativeSleep, UntilOrTimeoutReturnsTrueImmediatelyWhenPredicateAlreadyTrue) {
	gStepInvocationCount = 0;
	const bool result = CooperativeSleepUntilOrTimeout([]() { return true; }, /*timeoutMs=*/5);
	EXPECT_TRUE(result);
	EXPECT_EQ(gStepInvocationCount, 0);
}

TEST(CooperativeSleep, UntilOrTimeoutReturnsFalseWhenPredicateStaysFalse) {
	gStepInvocationCount = 0;
	// The predicate never becomes true; with a tiny timeout the call must give up and return false.
	const bool result = CooperativeSleepUntilOrTimeout(
			[]() { return false; }, /*timeoutMs=*/3, /*pollIntervalMs=*/1);
	EXPECT_FALSE(result);
	// The polling loop ran at least once before giving up.
	EXPECT_GT(gStepInvocationCount, 0);
}
