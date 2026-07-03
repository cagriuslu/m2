#include <gtest/gtest.h>
#include <m2/mt/ProtectedObject.h>
#include <chrono>

using namespace m2;

TEST(ProtectedObject, ReadReturnsWrappedValue) {
	ProtectedObject<int> obj{42};
	int seen = 0;
	obj.Read([&](const int& value) { seen = value; });
	EXPECT_EQ(seen, 42);
}

TEST(ProtectedObject, WriteMutatesWrappedValue) {
	ProtectedObject<int> obj{1};
	obj.Write([](int& value) { value = 100; });

	int seen = 0;
	obj.Read([&](const int& value) { seen = value; });
	EXPECT_EQ(seen, 100);
}

TEST(ProtectedObject, WaitUntilAndReadReturnsImmediatelyWhenConditionAlreadyTrue) {
	ProtectedObject<int> obj{7};
	int seen = 0;
	// The condition is already satisfied, so the reader is invoked without blocking.
	const bool result = obj.WaitUntilAndRead(
			[](const int& value) { return value == 7; },
			[&](const int& value) { seen = value; });
	EXPECT_TRUE(result);
	EXPECT_EQ(seen, 7);
}

TEST(ProtectedObject, WaitUntilAndReadTimesOutWhenConditionNeverTrue) {
	ProtectedObject<int> obj{0};
	bool readerCalled = false;
	// The condition never holds; with a tiny timeout the call must return false and skip the reader.
	const bool result = obj.WaitUntilAndRead(
			[](const int& value) { return value == 999; },
			[&](const int&) { readerCalled = true; },
			std::chrono::milliseconds{5});
	EXPECT_FALSE(result);
	EXPECT_FALSE(readerCalled);
}

TEST(ProtectedObject, WaitUntilAndWriteReturnsImmediatelyWhenConditionAlreadyTrue) {
	ProtectedObject<int> obj{5};
	const bool result = obj.WaitUntilAndWrite(
			[](const int& value) { return value == 5; },
			[](int& value) { value = 6; });
	EXPECT_TRUE(result);

	int seen = 0;
	obj.Read([&](const int& value) { seen = value; });
	EXPECT_EQ(seen, 6);
}

TEST(ProtectedObject, WaitUntilAndWriteTimesOutWhenConditionNeverTrue) {
	ProtectedObject<int> obj{0};
	bool writerCalled = false;
	const bool result = obj.WaitUntilAndWrite(
			[](const int&) { return false; },
			[&](int&) { writerCalled = true; },
			std::chrono::milliseconds{5});
	EXPECT_FALSE(result);
	EXPECT_FALSE(writerCalled);
}
