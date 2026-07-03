#include <gtest/gtest.h>
#include <m2/thirdparty/protobuf/Deserialize.h>
#include <m2/thirdparty/protobuf/Compare.h>
#include <Key.pb.h>

using namespace m2;

TEST(ThirdpartyProtobufToMessage, RoundTripsSerializedMessage) {
	pb::Key original;
	original.add_sdl_scancodes(7);
	original.add_sdl_scancodes(42);

	const std::string serialized = original.SerializeAsString();
	const std::vector<uint8_t> bytes(serialized.begin(), serialized.end());

	const auto roundTripped = thirdparty::protobuf::ToMessage<pb::Key>(bytes);

	ASSERT_TRUE(roundTripped.has_value());
	EXPECT_TRUE(thirdparty::protobuf::IsEqual(original, *roundTripped));
}

TEST(ThirdpartyProtobufToMessage, TruncatedBytesYieldNullopt) {
	// Tag 0x12: field 2 (sdl_scancodes), length-delimited. Length 0x0A claims 10 payload bytes,
	// but only a single byte follows -> malformed wire format that must fail to parse.
	const std::vector<uint8_t> truncatedBytes{0x12, 0x0A, 0x01};

	const auto result = thirdparty::protobuf::ToMessage<pb::Key>(truncatedBytes);

	EXPECT_FALSE(result.has_value());
}
