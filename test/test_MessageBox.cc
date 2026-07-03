#include <gtest/gtest.h>
#include <m2/mt/actor/MessageBox.h>
#include <optional>
#include <vector>

using namespace m2;

TEST(MessageBox, PeekReturnsFrontMostInFifoOrder) {
	MessageBox<int> box;
	box.PushMessage(1);
	box.PushMessage(2);
	box.PushMessage(3);

	const int* front = box.PeekMessage();
	ASSERT_NE(front, nullptr);
	EXPECT_EQ(*front, 1);
	// Peeking does not pop; the front stays the same.
	EXPECT_EQ(*box.PeekMessage(), 1);
}

TEST(MessageBox, PeekOnEmptyReturnsNull) {
	MessageBox<int> box;
	EXPECT_EQ(box.PeekMessage(), nullptr);
}

TEST(MessageBox, TryPopReturnsMessagesInFifoOrder) {
	MessageBox<int> box;
	box.PushMessage(10);
	box.PushMessage(20);

	std::optional<int> out;
	ASSERT_TRUE(box.TryPopMessage(out));
	EXPECT_EQ(out, 10);
	ASSERT_TRUE(box.TryPopMessage(out));
	EXPECT_EQ(out, 20);
}

TEST(MessageBox, TryPopOnEmptyReportsEmpty) {
	MessageBox<int> box;
	std::optional<int> out;
	EXPECT_FALSE(box.TryPopMessage(out));
	EXPECT_FALSE(out.has_value());
}

TEST(MessageBox, TryHandleMessagesRespectsBoundThenExitsOnEmpty) {
	MessageBox<int> box;
	box.PushMessage(1);
	box.PushMessage(2);
	box.PushMessage(3);

	int handledCount = 0;
	int sum = 0;
	// Bounded to 2: only the first two messages are handled.
	box.TryHandleMessages([&](const int& value) { ++handledCount; sum += value; }, 2);
	EXPECT_EQ(handledCount, 2);
	EXPECT_EQ(sum, 3);

	// Unbounded: handles the remaining message and stops when the queue is empty.
	box.TryHandleMessages([&](const int& value) { ++handledCount; sum += value; });
	EXPECT_EQ(handledCount, 3);
	EXPECT_EQ(sum, 6);
	EXPECT_EQ(box.PeekMessage(), nullptr);
}

TEST(MessageBox, TryHandleMessagesExitsEarlyWhenQueueEmptiesBeforeBound) {
	MessageBox<int> box;
	box.PushMessage(1);
	box.PushMessage(2);

	int handledCount = 0;
	// Bound of 5 exceeds available; only the 2 present messages are handled.
	box.TryHandleMessages([&](const int&) { ++handledCount; }, 5);
	EXPECT_EQ(handledCount, 2);
}

TEST(MessageBox, TryHandleMessagesUntilStopsWhenHandlerReturnsFalse) {
	MessageBox<int> box;
	box.PushMessage(1);
	box.PushMessage(2);
	box.PushMessage(3);

	std::vector<int> handled;
	// Handler stops the loop when it sees 2, leaving 3 in the queue.
	box.TryHandleMessagesUntil([&](const int& value) {
		handled.push_back(value);
		return value != 2;
	});
	ASSERT_EQ(handled.size(), 2u);
	EXPECT_EQ(handled[0], 1);
	EXPECT_EQ(handled[1], 2);

	std::optional<int> out;
	ASSERT_TRUE(box.TryPopMessage(out));
	EXPECT_EQ(out, 3);
}

TEST(MessageBox, TryHandleMessagesUntilHandlesAllWhenHandlerKeepsReturningTrue) {
	MessageBox<int> box;
	box.PushMessage(4);
	box.PushMessage(5);

	int sum = 0;
	box.TryHandleMessagesUntil([&](const int& value) { sum += value; return true; });
	EXPECT_EQ(sum, 9);
	EXPECT_EQ(box.PeekMessage(), nullptr);
}

TEST(SendQuestionReceiveAnswerSync, DispatchesUninterestingThenHandlesInterestingAnswer) {
	MessageBox<int> sendBox;
	MessageBox<int> recvBox;
	// Pre-populate the receive box so the internal WaitMessage returns immediately (deterministic, non-blocking).
	recvBox.PushMessage(10); // uninteresting
	recvBox.PushMessage(20); // interesting

	int interestingValue = 0;
	std::vector<int> uninterestingValues;
	SendQuestionReceiveAnswerSync<int, int>(
			sendBox, 5, recvBox,
			[](const int& value) { return value == 20; },
			[&](const int& value) { interestingValue = value; },
			[&](const int& value) { uninterestingValues.push_back(value); });

	// The question was pushed onto the send box.
	const int* sent = sendBox.PeekMessage();
	ASSERT_NE(sent, nullptr);
	EXPECT_EQ(*sent, 5);

	// The uninteresting response was dispatched before the interesting one was handled.
	ASSERT_EQ(uninterestingValues.size(), 1u);
	EXPECT_EQ(uninterestingValues[0], 10);
	EXPECT_EQ(interestingValue, 20);

	// Both responses were consumed once the interesting one stopped the loop.
	EXPECT_EQ(recvBox.PeekMessage(), nullptr);
}
