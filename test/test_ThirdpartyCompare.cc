#include <gtest/gtest.h>
#include <m2/thirdparty/protobuf/Compare.h>
#include <Key.pb.h>

using namespace m2;

TEST(ThirdpartyProtobufIsEqual, EqualMessagesCompareEqual) {
	pb::Key first;
	first.add_sdl_scancodes(3);
	first.add_sdl_scancodes(9);

	pb::Key second;
	second.add_sdl_scancodes(3);
	second.add_sdl_scancodes(9);

	EXPECT_TRUE(thirdparty::protobuf::IsEqual(first, second));
}

TEST(ThirdpartyProtobufIsEqual, DifferingMessagesCompareUnequal) {
	pb::Key first;
	first.add_sdl_scancodes(3);

	pb::Key second;
	second.add_sdl_scancodes(4);

	EXPECT_FALSE(thirdparty::protobuf::IsEqual(first, second));
}
