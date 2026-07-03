#include <gtest/gtest.h>
#include <m2/protobuf/Detail.h>
#include <VecI.pb.h>
#include <RectI.pb.h>

using namespace m2;

TEST(ProtobufJsonString, RoundTripPreservesFields) {
	pb::VecI original;
	original.set_x(3);
	original.set_y(-5);

	const auto json = pb::message_to_json_string(original);
	ASSERT_TRUE(json.has_value());

	const auto parsed = pb::json_string_to_message<pb::VecI>(*json);
	ASSERT_TRUE(parsed.has_value());
	EXPECT_EQ(parsed->x(), 3);
	EXPECT_EQ(parsed->y(), -5);
}

TEST(ProtobufJsonString, RoundTripMultipleFields) {
	pb::RectI original;
	original.set_x(1);
	original.set_y(2);
	original.set_w(30);
	original.set_h(40);

	const auto json = pb::message_to_json_string(original);
	ASSERT_TRUE(json.has_value());

	const auto parsed = pb::json_string_to_message<pb::RectI>(*json);
	ASSERT_TRUE(parsed.has_value());
	EXPECT_EQ(parsed->x(), 1);
	EXPECT_EQ(parsed->y(), 2);
	EXPECT_EQ(parsed->w(), 30);
	EXPECT_EQ(parsed->h(), 40);
}

TEST(ProtobufJsonString, InvalidJsonReturnsError) {
	const auto parsed = pb::json_string_to_message<pb::VecI>("{ this is not valid json");
	EXPECT_FALSE(parsed.has_value());
}
