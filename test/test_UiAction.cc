#include <gtest/gtest.h>
#include <m2/ui/UiAction.h>
#include <string>

using namespace m2;

TEST(UiAction, ContinueIsNotAReturn) {
	const auto action = MakeContinueAction();
	EXPECT_TRUE(action.IsContinue());
	EXPECT_FALSE(action.IsReturn());
	EXPECT_FALSE(action.IsReturn<int>());
	EXPECT_FALSE(action.IsVoidReturn());
}

TEST(UiAction, ReturnDistinguishesValueType) {
	const auto action = MakeReturnAction(42);
	EXPECT_FALSE(action.IsContinue());
	EXPECT_TRUE(action.IsReturn());
	EXPECT_TRUE(action.IsReturn<int>());
	// A Return<int> is not a Return<float> nor a Return<Void>.
	EXPECT_FALSE(action.IsReturn<float>());
	EXPECT_FALSE(action.IsVoidReturn());
}

TEST(UiAction, VoidReturnIsRecognized) {
	const auto action = MakeReturnAction();
	EXPECT_TRUE(action.IsReturn());
	EXPECT_TRUE(action.IsVoidReturn());
	EXPECT_TRUE(action.IsReturn<Void>());
	EXPECT_FALSE(action.IsReturn<int>());
}

TEST(UiAction, IfContinueDispatchesOnlyContinueBranch) {
	const auto action = MakeContinueAction();
	bool continueCalled = false;
	bool returnCalled = false;
	action.IfContinue([&] { continueCalled = true; })
		.IfReturn<int>([&](const int&) { returnCalled = true; });
	EXPECT_TRUE(continueCalled);
	EXPECT_FALSE(returnCalled);
}

TEST(UiAction, IfReturnDispatchesWithValue) {
	const auto action = MakeReturnAction(7);
	bool continueCalled = false;
	int capturedValue = 0;
	action.IfContinue([&] { continueCalled = true; })
		.IfReturn<int>([&](const int& value) { capturedValue = value; });
	EXPECT_FALSE(continueCalled);
	EXPECT_EQ(capturedValue, 7);
}

TEST(UiAction, IfVoidReturnDispatchesOnlyVoidBranch) {
	const auto action = MakeReturnAction();
	bool voidCalled = false;
	bool intCalled = false;
	action.IfVoidReturn([&] { voidCalled = true; })
		.IfReturn<int>([&](const int&) { intCalled = true; });
	EXPECT_TRUE(voidCalled);
	EXPECT_FALSE(intCalled);
}

TEST(UiAction, ContinueFocusStates) {
	// No focus preference by default.
	const Continue noPreference;
	EXPECT_FALSE(noPreference.Focus().has_value());

	// Requesting focus.
	const Continue requestFocus{true};
	ASSERT_TRUE(requestFocus.Focus().has_value());
	EXPECT_TRUE(*requestFocus.Focus());

	// Yielding focus.
	const Continue yieldFocus{false};
	ASSERT_TRUE(yieldFocus.Focus().has_value());
	EXPECT_FALSE(*yieldFocus.Focus());
}

TEST(UiAction, IfContinueWithFocusStateInvokesHandlerWhenFocusSet) {
	const auto action = MakeContinueAction(true);
	std::optional<bool> receivedFocus;
	action.IfContinueWithFocusState([&](const bool focus) { receivedFocus = focus; });
	ASSERT_TRUE(receivedFocus.has_value());
	EXPECT_TRUE(*receivedFocus);
}

TEST(UiAction, IfContinueWithFocusStateSkipsHandlerWhenFocusUnset) {
	const auto action = MakeContinueAction();  // no focus preference
	bool handlerCalled = false;
	action.IfContinueWithFocusState([&](bool) { handlerCalled = true; });
	EXPECT_FALSE(handlerCalled);
}

TEST(UiAction, ReturnMovesHeldValue) {
	std::string original = "hello world";
	const auto action = MakeReturnAction(std::move(original));
	ASSERT_TRUE(action.IsReturn<std::string>());

	bool handlerCalled = false;
	action.IfReturn<std::string>([&](const std::string& value) {
		handlerCalled = true;
		EXPECT_EQ(value, "hello world");
	});
	EXPECT_TRUE(handlerCalled);
}

TEST(UiAction, ExtractAnyReturnContainerMovesOutTheReturn) {
	auto action = MakeReturnAction(99);
	auto extracted = action.ExtractAnyReturnContainer();
	ASSERT_TRUE(extracted.has_value());

	const auto* returnPtr = dynamic_cast<const Return<int>*>(extracted->Get());
	ASSERT_NE(returnPtr, nullptr);
	EXPECT_EQ(returnPtr->Value(), 99);
}

TEST(UiAction, ConvertReturnActionToContinueOnlyConvertsReturns) {
	// A Return becomes a Continue.
	auto returnAction = MakeReturnAction(5);
	const auto converted = ConvertReturnActionToContinue(std::move(returnAction));
	EXPECT_TRUE(converted.IsContinue());

	// A Continue passes through unchanged.
	auto continueAction = MakeContinueAction();
	const auto passthrough = ConvertReturnActionToContinue(std::move(continueAction));
	EXPECT_TRUE(passthrough.IsContinue());
}
