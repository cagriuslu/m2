#include <gtest/gtest.h>
#include <m2/protobuf/MessageLUT.h>
#include <Key.pb.h>

using namespace m2;

namespace {
	// A minimal LoadedItemT so that MessageLUT<pb::Key, ...>::load and operator[] can be exercised.
	// MessageLUT::load constructs each loaded item from the corresponding proto item.
	struct LoadedKey {
		m2g::pb::KeyType type{};
		LoadedKey() = default;
		explicit LoadedKey(const pb::Key& key) : type(key.type()) {}
	};

	// A non-existent override path forces MessageLUT to fall back to the in-memory envelope.
	const std::filesystem::path kNonExistentOverridePath{"/nonexistent/m2-test-keys-override.json"};

	// Builds a Keys envelope with exactly one Key per KeyType enum value (a complete definition).
	pb::Keys MakeCompleteKeysEnvelope() {
		pb::Keys envelope;
		pb::for_each_enum_value<m2g::pb::KeyType>([&envelope](const m2g::pb::KeyType keyType) {
			envelope.add_keys()->set_type(keyType);
		});
		return envelope;
	}
}

TEST(MessageLUT, LoadProtoItemsCompleteEnvelopeSucceeds) {
	const auto envelope = MakeCompleteKeysEnvelope();

	const auto protoItems =
			pb::MessageLUT<pb::Key>::LoadProtoItems(&envelope, kNonExistentOverridePath, &pb::Keys::keys);

	ASSERT_TRUE(protoItems.has_value());
	EXPECT_EQ(static_cast<int>(protoItems->size()), pb::enum_value_count<m2g::pb::KeyType>());
}

TEST(MessageLUT, LoadProtoItemsMissingEntryIsError) {
	const pb::Keys emptyEnvelope; // No Key defined for any enum value.

	const auto protoItems =
			pb::MessageLUT<pb::Key>::LoadProtoItems(&emptyEnvelope, kNonExistentOverridePath, &pb::Keys::keys);

	// A missing definition must be surfaced as an error, not silently defaulted.
	ASSERT_FALSE(protoItems.has_value());
	EXPECT_NE(protoItems.error().find("not defined"), std::string::npos);
}

TEST(MessageLUT, LoadProtoItemsDuplicateEntryIsError) {
	auto envelope = MakeCompleteKeysEnvelope();
	// Add a second definition for the first enum value, creating a duplicate.
	envelope.add_keys()->set_type(pb::enum_value<m2g::pb::KeyType>(0));

	const auto protoItems =
			pb::MessageLUT<pb::Key>::LoadProtoItems(&envelope, kNonExistentOverridePath, &pb::Keys::keys);

	ASSERT_FALSE(protoItems.has_value());
	EXPECT_NE(protoItems.error().find("duplicate"), std::string::npos);
}

TEST(MessageLUT, LoadExposesItemsViaSubscript) {
	const auto envelope = MakeCompleteKeysEnvelope();

	const auto lut =
			pb::MessageLUT<pb::Key, LoadedKey>::load(&envelope, kNonExistentOverridePath, &pb::Keys::keys);

	ASSERT_EQ(static_cast<int>(lut.size()), pb::enum_value_count<m2g::pb::KeyType>());
	// Each enum value must map to the loaded item built from its proto definition.
	const auto firstKeyType = pb::enum_value<m2g::pb::KeyType>(0);
	EXPECT_EQ(lut[firstKeyType].type, firstKeyType);
	const auto lastKeyType = pb::enum_value<m2g::pb::KeyType>(pb::enum_value_count<m2g::pb::KeyType>() - 1);
	EXPECT_EQ(lut[lastKeyType].type, lastKeyType);
}
