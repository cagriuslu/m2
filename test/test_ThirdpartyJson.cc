#include <gtest/gtest.h>
#include <m2/thirdparty/protobuf/Json.h>
#include <Key.pb.h>

using namespace m2;

TEST(ThirdpartyProtobufToJsonString, ContainsSetFieldValue) {
	pb::Key key;
	key.add_sdl_scancodes(12345);

	const auto json = thirdparty::protobuf::ToJsonString(key);

	ASSERT_TRUE(json.has_value());
	EXPECT_NE(json->find("12345"), std::string::npos);
}
